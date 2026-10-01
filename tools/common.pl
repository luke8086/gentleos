#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: common.pl - Shared helpers
#

use strict;

sub min {
    my ($x, $y) = @_;
    return $x < $y ? $x : $y;
}

sub max {
    my ($x, $y) = @_;
    return $x > $y ? $x : $y;
}

sub slurp {
    my ($path) = @_;
    open(my $f, "<", $path) or die "Cannot read $path: $!\n";
    binmode $f;
    local $/;
    my $data = <$f>;
    close $f;
    return $data;
}

sub spit {
    my ($path, $data) = @_;
    open(my $f, ">", $path) or die "Cannot write $path: $!\n";
    binmode $f;
    print $f $data;
    close $f or die "Write error on $path\n";
}

1;
