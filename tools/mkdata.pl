#!/usr/bin/perl
#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mkdata.pl - Convert bitmaps and fonts to hardcoded C data
#

use File::Basename;

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

sub process_all {
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

}

process_all();
