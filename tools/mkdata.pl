#!/usr/bin/perl
#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mkdata.pl - Generate data.c and initrd
#

use strict;

use File::Basename;

require "./tools/common.pl";

my $INITRD_MAGIC        = "IRD2";
my $INITRD_VERSION      = 2;
my $INITRD_NAME_LEN     = 31;
my $INITRD_HEADER_LEN   = 12;                   # a4 magic + V version + V count
my $INITRD_ENTRY_LEN    = $INITRD_NAME_LEN + 9; # name + C type + V offset + V size
my $INITRD_MAX_SIZE     = 0x20000; # 128KB, must match initrd.c
my $INITRD_PATH         = "gentleos.dat";

my $FILE_TYPE_UNKNOWN = 0;
my $FILE_TYPE_BITMAP  = 1;
my $FILE_TYPE_SONG    = 2;

my @FILE_TYPE_NAMES = (
    "unknown",
    "bitmap",
    "song",
);

my $FILE_MAX_SIZE = 0xfff0;    # Must be addressable through one far pointer

my $SONG_TICK_FREQUENCY = 100; # Must match lib.h

my @FONTS = (
    {
        path => "vendor/int10h/evxme58.pbm",
        name => "Evx ME 5x8",
        width => 5,
        height => 8,
        pitch => 8,
    },
);

my $FONT_MAX_CHARS = 128;

sub clean_pbm {
    my ($path) = @_;

    open(my $fh, "<", $path) or die "Cannot read $path: $!\n";
    my @lines = grep { !/^#/ } <$fh>;
    close $fh;

    open($fh, ">", $path) or die "Cannot write $path: $!\n";
    binmode($fh);
    print $fh @lines;
    close $fh;
}

sub load_pbm {
    my ($path) = @_;
    open(my $fh, "<", $path) or die "Cannot read $path: $!\n";

    printf "- %-28s", $path;

    my @header;
    my $raster = "";
    while (my $line = <$fh>) {
        $line =~ s/#.*$//;
        if (@header < 3) {
            my @parts = split(' ', $line);
            while (@parts && @header < 3) {
                push @header, shift @parts;
            }
            $raster .= join("", @parts);
        } else {
            $raster .= $line;
        }
    }
    close $fh;

    if (@header < 3 || $header[0] ne "P1") {
        die "\nNot a P1 PBM file\n";
    }


    my $width = int($header[1]);
    my $height = int($header[2]);

    print "  size: ${width}x${height}";

    $raster =~ s/\s+//g;
    my @flat = split(//, $raster);

    my @pixels;
    for (my $i = 0; $i < @flat; $i += $width) {
        push @pixels, [@flat[$i .. $i + $width - 1]];
    }

    return (\@pixels, $width, $height);
}

sub bitmap_name {
    my ($p) = @_;
    $p =~ s{.*[/\\]}{};
    $p =~ s{\.[^.]*$}{};
    return $p;
}

sub process_bitmap {
    my ($path) = @_;

    clean_pbm($path);

    my $name = bitmap_name($path);
    my $dirname = dirname($path);

    my ($pixels, $width, $height) = load_pbm($path);
    my $pitch = int(($width + 7) / 8);

    print "\n";

    my @pixel_lines;

    foreach my $row (@$pixels) {
        my @bytes;
        for (my $i = 0; $i < @$row; $i += 8) {
            my $byte = 0;
            for (my $bit_pos = 0; $bit_pos < 8; $bit_pos++) {
                if ($i + $bit_pos < @$row) {
                    $byte |= $row->[$i + $bit_pos] << (7 - $bit_pos);
                }
            }
            push @bytes, $byte;
        }

        my $pixel_str = join("", map { sprintf("\\x%02x", $_) } @bytes);
        push @pixel_lines, "        \"$pixel_str\" \\";
    }

    my $prefix = "bitmap_";
    $prefix = "icon_" if $dirname eq "assets/icons";
    $prefix = "icon_" if $dirname eq "vendor/icons8";
    $prefix = "card_" if $dirname eq "assets/cards";
    $prefix = "sprite_" if $dirname eq "assets/sprites";
    $prefix = "sprite_mj_" if $dirname eq "assets/mahjong";
    $prefix = "glyph_mn_" if $dirname eq "vendor/mona";

    my @lines = (
        "global bitmap_st $prefix$name = {",
        "    { $width, $height },",
        "    $pitch,",
        "    (uint8_t *)",
        @pixel_lines,
        "};",
        "",
    );

    return join("\r\n", @lines);
}

sub process_bitmaps {
    my @bitmap_files = sort((
        glob("bitmaps/*.pbm"),
        glob("assets/icons/*.pbm"),
        glob("assets/cards/*.pbm"),
        glob("assets/mahjong/*.pbm"),
        glob("assets/sprites/*.pbm"),
        glob("vendor/icons8/*.pbm"),
        glob("vendor/mona/*.pbm"),
    ));

    my @lines = map { process_bitmap($_) } @bitmap_files;

    return join("\r\n", @lines)
}

sub load_font {
    my ($font) = @_;
    my $path = $font->{path};
    my $width = $font->{width};
    my $height = $font->{height};
    my $pitch = $font->{pitch};

    my ($pixels, $img_width, $img_height) = load_pbm($path);

    my $cols = int($img_width / $pitch);
    my $rows = int($img_height / $height);
    my $num_chars = $cols * $rows;

    print "  grid: ${cols}x${rows}  chars: $num_chars\n";

    if ($num_chars > $FONT_MAX_CHARS) {
        $num_chars = $FONT_MAX_CHARS;
    }
    my $max_bytes = $FONT_MAX_CHARS * $height;

    my @glyph_bytes;
    for (my $ch = 0; $ch < $num_chars; $ch++) {
        my $col = $ch % $cols;
        my $row = int($ch / $cols);
        my $x_start = $col * $pitch;
        my $y_start = $row * $height;

        for (my $j = 0; $j < $height; $j++) {
            my $byte = 0;
            for (my $i = 0; $i < $pitch; $i++) {
                my $x = $x_start + $i;
                my $y = $y_start + $j;
                my $p = $pixels->[$y][$x];
                $byte |= $p << (7 - $i);
            }
            push @glyph_bytes, $byte;
        }
    }

    while (@glyph_bytes < $max_bytes) {
        push @glyph_bytes, 0;
    }

    return [@glyph_bytes[0 .. $max_bytes - 1]];
}

sub format_font_pixels {
    my ($glyph_bytes, $height) = @_;
    my @lines;
    my $num_chars = int(@$glyph_bytes / $height);

    for (my $i = 0; $i < $num_chars; $i++) {
        my $offset = $i * $height;
        my @chunk = @{$glyph_bytes}[$offset .. $offset + $height - 1];
        my $hex_str = join("", map { sprintf("\\x%02x", $_) } @chunk);
        push @lines, "            \"$hex_str\" \\";
    }

    return join("\r\n", @lines);
}

sub process_fonts {
    my @font_data;
    foreach my $font (@FONTS) {
        my $glyph_bytes = load_font($font);
        push @font_data, [$font, $glyph_bytes];
    }

    my @lines = (
        "global font_st fonts[] = {",
    );

    foreach my $entry (@font_data) {
        my ($font, $glyph_bytes) = @$entry;
        my $name = $font->{name};
        my $width = $font->{width};
        my $height = $font->{height};
        push @lines, (
            "    {",
            "        { $width, $height },",
            "        \"$name\",",
            "        (uint8_t *)",
            format_font_pixels($glyph_bytes, $height),
            "    },",
        );
    }

    push @lines, "};";

    return join("\r\n", @lines);

}

sub read_spk {
    my ($path) = @_;
    my %meta;
    my @segments;

    open(my $fh, "<", $path) or die "Cannot read $path: $!\n";

    while (my $line = <$fh>) {
        next if $line =~ /^\s*$/;

        if ($line =~ /^\s*(\w+)\s*:\s*(.+?)\s*$/) {
            $meta{$1} = $2;
            next;
        }

        if ($line =~ /^\s*(\d+)\s*,\s*(\d+)\s*$/) {
            my $pitch = min(int($1), 0xffff);
            my $duration = max(min(int($2), 0xffff), 1);

            push @segments, [$pitch, $duration];
            next;
        }

        die "Error: $path:$.: invalid syntax\n";
    }

    close $fh;

    die "Error: no title in $path\n" if $meta{title} eq "";
    die "Error: no notes in $path\n" if !@segments;

    return ($meta{title}, \@segments);
}

sub process_spk {
    my ($path) = @_;

    print "Importing $path... ";

    my ($title, $segments) = read_spk($path);

    my $data = "";
    my $total_ms = 0;
    my $total_ticks = 0;

    foreach my $segment (@$segments) {
        my ($pitch, $duration) = @$segment;

        $total_ms += $duration;
        my $ticks = int(($total_ms * $SONG_TICK_FREQUENCY + 500) / 1000) - $total_ticks;
        $ticks = max(min($ticks, 0xffff), 1);
        $total_ticks += $ticks;

        $data .= pack("vv", $pitch, $ticks);
    }

    $data .= pack("vv", 0, 0);

    printf "ok (\"%s\", %d notes, %d ms)\n", $title, scalar(@$segments), $total_ms;

    return {
        name => substr($title, 0, $INITRD_NAME_LEN - 1),
        type => $FILE_TYPE_SONG,
        data => $data,
    };
}

sub build_initrd_image {
    my (@files) = @_;

    my $count = scalar(@files);
    my $offset = $INITRD_HEADER_LEN + $count * $INITRD_ENTRY_LEN;
    my $table = "";
    my $blobs = "";

    foreach my $file (@files) {
        my $name = $file->{name};
        my $size = length($file->{data});
        my $ftype = $file->{type};

        if ($size > $FILE_MAX_SIZE) {
            die "Error: file \"$name\" is too big ($size > $FILE_MAX_SIZE bytes)\n";
        }

        printf "- %s: %x (%u B, %s)\n", $name, $offset, $size, $FILE_TYPE_NAMES[$ftype];

        $table .= pack("a${INITRD_NAME_LEN} C V V", $name, $ftype, $offset, $size);
        $blobs .= $file->{data};
        $offset += $size;
    }

    return pack("a4 V V", $INITRD_MAGIC, $INITRD_VERSION, $count) . $table . $blobs;
}

sub make_initrd {
    print "\nImporting initrd assets:\n";
    my @files = map { process_spk($_) } sort(glob("assets/spk/*.spk"));

    die "Error: no songs found\n" if !@files;

    print "\nGenerating initrd:\n";
    my $image = build_initrd_image(@files);
    my $size = length($image);

    if ($size > $INITRD_MAX_SIZE) {
        die "Error: initrd is too big ($size > $INITRD_MAX_SIZE bytes)\n";
    }

    spit($INITRD_PATH, $image);

    print "Initrd saved to $INITRD_PATH ($size bytes)\n";
}

sub make_data {
    print "Generating data.c:\n";

    my $bitmap_lines = process_bitmaps();
    my $font_lines = process_fonts();

    my @lines = (
        "#include <gui.h>",
        "",
        $bitmap_lines,
        $font_lines,
        ""
    );

    -d "data" or mkdir "data" or die "Cannot create data dir: $!\n";
    open(my $fh, ">", "data/data.c") or die "Cannot write data/data.c: $!\n";
    binmode($fh);
    print $fh join("\r\n", @lines);
    close($fh);

    print "Static data saved to data/data.c\n";


}

make_data();
make_initrd();
