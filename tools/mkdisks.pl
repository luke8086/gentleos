#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mkdisks.pl - Script for creating disk images
#

require "./tools/common.pl";

my $KRN_FLAG_COLORS_INVERTED = (1 << 1);

my $KERNEL_SIZE = 127 * 512;
my $INITRD_SIZE = 256 * 512;

sub pad {
    my ($data, $size) = @_;

    return $data if $size == 0;

    my $data_size = length($data);

    die "Data exceeds padded size ($data_size > $size)\n" if $data_size > $size;

    return $data . "\0" x ($size - $data_size);
}

sub make_disk {
    my ($path, $size, $flags) = @_;

    print "Creating $path... ";

    my $boot1 = pad(slurp("build/boot1/boot1.bin"), 512);
    my $boot2 = pad(slurp("build/boot2/boot2.com"), 2048);

    my $kernel = slurp("GT16.COM");
    substr($kernel, 2, 2, pack("v", $flags));
    $kernel = pad($kernel, $KERNEL_SIZE);

    my $initrd = pad(slurp("GT16.DAT"), $INITRD_SIZE);

    my $image = pad($boot1 . $boot1 . $boot2 . $kernel . $initrd, $size);

    spit($path, $image);

    print "Done\n";
}

make_disk("GT16DISK.IMG", 0, 0x00);
make_disk("GT16FD72.IMG", 720 * 1024, 0x00);
make_disk("GT16FD14.IMG", 1440 * 1024, 0x00);
make_disk("GT16WEB.IMG", 1440 * 1024, $KRN_FLAG_COLORS_INVERTED);
