#!/usr/bin/perl
use strict;

my ($test_out) = @ARGV;

if (@ARGV != 1) {
    print STDERR "Invalid usage\n";
    exit 1;
}

my $ln_num = 1;
open(my $f, $test_out) or die $!;
while (<$f>) {
    my $re = $_;
    my $ln = <STDIN>;

    if ($ln !~ $re) {
        chomp $ln;
        print STDERR "Assertion: ".$test_out."[".$ln_num."] != \"".$ln."\"\n";
        exit 1;
    }
    $ln_num++;
}
close($f)
