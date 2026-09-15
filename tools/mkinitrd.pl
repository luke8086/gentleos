#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mkinitrd.pl - Create initial RAM disk
#

my $MAGIC       = "IRD2";
my $VERSION     = 2;
my $NAME_LEN    = 31;
my $HEADER_LEN  = 12;                # a4 magic + V version + V count
my $ENTRY_LEN   = $NAME_LEN + 9;      # name + C type + V offset + V size

my $FILE_TYPE_UNKNOWN = 0;
my $FILE_TYPE_BITMAP  = 1;
my $FILE_TYPE_SONG    = 2;

my @FILE_TYPE_NAMES = (
    "unknown",
    "bitmap",
    "song",
);

my $PLAYER_TIMER_HZ = 100;
my $OUTPUT_MAX_SIZE = 0x10000; # 64KB
my $OUTPUT_PATH = "gentleos.dat";

sub min {
    my ($x, $y) = @_;
    return $x < $y ? $x : $y;
}

sub max {
    my ($x, $y) = @_;
    return $x > $y ? $x : $y;
}

sub spit {
    my ($path, $data) = @_;
    open(my $f, ">", $path) or die "Cannot write $path: $!\n";
    binmode $f;
    print $f $data;
    close $f or die "Write error on $path\n";
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
        my $ticks = int(($total_ms * $PLAYER_TIMER_HZ + 500) / 1000) - $total_ticks;
        $ticks = max(min($ticks, 0xffff), 1);
        $total_ticks += $ticks;

        $data .= pack("vv", $pitch, $ticks);
    }

    $data .= pack("vv", 0, 0);

    printf "ok (\"%s\", %d notes, %d ms)\n", $title, scalar(@$segments), $total_ms;

    return {
        name => substr($title, 0, $NAME_LEN - 1),
        type => $FILE_TYPE_SONG,
        data => $data,
    };
}

sub build_initrd {
    my (@files) = @_;

    my $count = scalar(@files);
    my $offset = $HEADER_LEN + $count * $ENTRY_LEN;
    my $table = "";
    my $blobs = "";

    foreach my $file (@files) {
        my $name = $file->{name};
        my $size = length($file->{data});
        my $ftype = $file->{type};

        printf "- %s: %x (%u B, %s)\n", $name, $offset, $size, $FILE_TYPE_NAMES[$ftype];

        $table .= pack("a${NAME_LEN} C V V", $name, $ftype, $offset, $size);
        $blobs .= $file->{data};
        $offset += $size;
    }

    return pack("a4 V V", $MAGIC, $VERSION, $count) . $table . $blobs;
}

sub main {
    my @files = map { process_spk($_) } sort(glob("assets/songs/*.spk"));

    die "Error: no songs found\n" if !@files;

    print "Generating initrd:\n";
    my $image = build_initrd(@files);
    my $size = length($image);

    if ($size > $OUTPUT_MAX_SIZE) {
        die "Error: initrd is too big ($size > $OUTPUT_MAX_SIZE bytes)\n";
    }

    spit($OUTPUT_PATH, $image);

    print "Initrd saved to $OUTPUT_PATH ($size bytes)\n";
}

main();
