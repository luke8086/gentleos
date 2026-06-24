#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mkcfg.pl - Script for creating config.h file
#

sub find_file {
    my ($name) = @_;

    opendir(my $d, ".") or die "Cannot read directory";
    my @match = grep { lc($_) eq lc($base) } readdir($d);
    closedir($d);

    return @match ? $match[0] : $name;
}

sub copy_file {
    my ($from, $to) = @_;

    open(my $in, "<", $from) or die "Cannot read $from: $!\n";
    binmode $in;
    local $/;
    my $data = <$in>;
    close $in;

    open(my $out, ">", $to) or die "Cannot write $to: $!\n";
    binmode $out;
    print $out $data;
    close $out or die "Write error on $to\n";
}

sub main {
    my $force = grep { $_ eq "-f" } @ARGV;

    print "Checking CONFIG.H... ";

    my $src = find_file("_config.h");
    my $dst = find_file("config.h");

    if (-f $dst && !$force) {
        print "already exists\n";
        return;
    }

    copy_file($src, $dst);
    print "created\n";
}

main();
