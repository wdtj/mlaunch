use Text::CSV;
use File::Basename;

my $csv = Text::CSV->new ( )  # should set binary attribute.
							 or die "Cannot use CSV: ".Text::CSV->error_diag ();

my %parts=();

foreach my $file (@ARGV)
{
	my $prefix=(fileparse($file, qr/\.[^.]*/))[0];
	open(FILE, "<$file") or die("Unable to open $file: $!");

	while(my $row = $csv->getline( *FILE ))
	{
		my ($ref, $quant, $name, $description, $package, $digikeyPart) = @{$row};
		if ($ref eq "Reference") {next;}

		my @refs=split(";", $ref);
		foreach my $x (0..$#refs)
		{
			$refs[$x]="$prefix-$refs[$x]";
		}

		$ref=join(" ", @refs);

		if (defined($parts{$digikeyPart}))
		{
	    my ($refs, $total)=@{$parts{$digikeyPart}};
			$refs=$refs." ".$ref;
			$total+=$quant;
			$parts{$digikeyPart}=[$refs, $total];
		}
		else
		{
			$parts{$digikeyPart}=[$ref, $quant];
		}
	}

	close(FILE);
}

for my $digikeyPart (sort keys %parts)
{
	my ($refs, $total)=@{$parts{$digikeyPart}};
	print("$refs,$total,$digikeyPart\n");
}
