#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mkdisks.pl - Script for creating disk images
#

my $KRN_FLAG_COLORS_INVERTED = (1 << 1);

my $KERNEL_SIZE = 127 * 512;
my $INITRD_SIZE = 128 * 512;

sub slurp {
    my ($path) = @_;
    open(my $f, $path) or die "Cannot read $path\n";
    binmode $f;
    local $/;
    my $data = <$f>;
    close $f;
    return $data;
}

sub pad {
    my ($data, $size) = @_;

    return $data if $size == 0;

    my $data_size = length($data);

    die "Data exceeds padded size ($data_size > $size)\n" if $data_size > $size;

    return $data . "\0" x ($size - $data_size);
}

sub make_disk {
    my ($path, $size, $flags) = @_;

    my $boot1 = pad(slurp("build/boot1/boot1.bin"), 512);
    my $boot2 = pad(slurp("build/boot2/boot2.com"), 2048);

    my $kernel = slurp("gentleos.com");
    substr($kernel, 2, 2, pack("v", $flags));
    $kernel = pad($kernel, $KERNEL_SIZE);

    my $initrd = pad("", $INITRD_SIZE);

    my $image = pad($boot1 . $boot1 . $boot2 . $kernel . $initrd, $size);

    print "Creating $path... ";

    unlink $path;
    open(my $out, ">$path") or die "Cannot write $path\n";
    binmode $out;
    print $out $image;
    close $out or die "Write error\n";

    print "Done\n";
}

make_disk("disk.img", 0, 0x00);
make_disk("fd720.img", 720 * 1024, 0x00);
make_disk("fd1440.img", 1440 * 1024, 0x00);
make_disk("web.img", 1440 * 1024, $KRN_FLAG_COLORS_INVERTED);
