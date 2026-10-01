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

sub update_file {
    my ($path, $data) = @_;

    if (!-e $path) {
        spit($path, $data);
        print "$path: created\n";
    } elsif (slurp($path) ne $data) {
        spit($path, $data);
        print "$path: updated\n";
    } else {
        print "$path: unchanged\n";
    }
}

1;
