#!/usr/bin/perl -w

    $NUM_SPACES = 2;

    open IN, $ARGV[0] or die;
    @Lines = <IN>;
    close IN;

    exit 0 unless ( $ARGV[0] =~ /\.(?:cpp|h)$/ );

    open TPL, "/root/c-smile/template.tpl" or die;
    $tpl = join "", <TPL>;
    close TPL;
    $tpl =~ s/##FILENAME##/$ARGV[0]/goi;

    open OUT, ">$ARGV[0]" or die;

    print OUT $tpl;

    $idx = 0;
    $identLevel = 0;

    while ( 1 )
    {
#print "Idx = $idx: $Lines[$idx]";
	chomp $Lines[$idx];
	$Lines[$idx] = beautyLines ( $Lines [ $idx ] );
	$Lines[$idx] =~ s/^\s+//;
	$Lines[$idx] =~ s/\s+$//;

	if ( $Lines[$idx] =~ /^\s*\/\// )
	{
	    $spaces = " " x ( $NUM_SPACES * $identLevel );
	    print OUT "$spaces$Lines[$idx]\n";
	}
	elsif ( $Lines[$idx] =~ /(.*?)(\{)([^\}]*)(\})\s*$/ )
	{
	    $Lines[$idx] = $1;
	    $Line = $3;
	    @aLine = split ';', $Line;
	    @aLine = map { s/^\s+//; $_ .= ';' if $_; } @aLine;
	    unshift @aLine, $2;
	    $aLine[$#aLine+1] = $4;
      @aLine = removeSpaces ( @aLine );

	    splice ( @Lines, $idx+1, 0, @aLine );
	    $idx--;
	}
	elsif ( $Lines[$idx] =~ /{/o )
	{
	    $line = $Lines[$idx];
	    $line =~ s/\"[^\"]+\"//;
	    $line =~ s/\'[^\']+\'//;

	    goto OTHERS if $line !~ /\{/;

	    $spaces = " " x ( $NUM_SPACES * $identLevel );

	    if ( $Lines[$idx] !~ /^\s*\{\s*(?:\/\/.*)?$/o )
	    {
		      my ( @aLine ) = $Lines[$idx] =~ /^(.+?)?\s*(\{)(.*)\s*$/o;
          @aLine = removeSpaces ( @aLine );
      		splice ( @Lines, $idx, 1 );
      		splice ( @Lines, $idx, 0, @aLine );
      		next;
	    }
	    else
	    {
      		print OUT "$spaces$Lines[$idx]\n";
	    }
      $identLevel++;
  }
  elsif ( $Lines[$idx] =~ /^\s*(?:case\s+|default\:|\S+\:\s*$|\S+\:\s*(?:\/\/))/io )
	{
	    $identLevel--;
	    $spaces = " " x ( $NUM_SPACES * $identLevel );
	    print OUT "$spaces$Lines[$idx]\n";
	    $identLevel++;
	}
	elsif ( $Lines[$idx] =~ /(?<![\'\"])\}(?![\'\"])/gio )
	{
    if ( $Lines[$idx] =~ /^\s*\};?\s*$/ )
    {
      $identLevel--;
      $spaces = " " x ( $NUM_SPACES * $identLevel );
      print OUT "$spaces$Lines[$idx]\n";
    }
    elsif ( $Lines[$idx] =~ /^(?:.+?)?\s*\};?\s*(?:.*?)?\s*$/ )
    {
      my ( @aLine ) = $Lines[$idx] =~ /^(.+?)?\s*(\};?)\s*(.*?)?\s*$/o;
      @aLine = removeSpaces ( @aLine );
      splice ( @Lines, $idx, 1 );
      splice ( @Lines, $idx, 0, @aLine );
      next;
    }
  }
  elsif ( $Lines[$idx] =~ /(?:^\s*else\s*$|\s*if\s*\([^;]+|\s*for\s*\(.+?\;\s*$|\s*while\s*\([^;]+)\s*$/goi && $Lines[$idx+1] !~ /^\s*{/ )
  {
    $spaces = " " x ( $NUM_SPACES * $identLevel );
    print OUT "$spaces$Lines[$idx]\n";
    $identLevel++;
    $spaces = " " x ( $NUM_SPACES * $identLevel );
    $idx++;
	  $Lines[$idx] = beautyLines ( $Lines [ $idx ] );
    $Lines[$idx] =~ s/^\s+//;
    $Lines[$idx] =~ s/\s+$//;
    print OUT "$spaces$Lines[$idx]\n";
    $identLevel--;
  }
  else
  {
OTHERS:
    if ( $Lines[$idx] && $Lines[$idx] !~ /^\s*#/ )
    {
      $spaces = " " x ( $NUM_SPACES * $identLevel );
      print OUT "$spaces$Lines[$idx]\n";
    }
    elsif ( $Lines[$idx] =~ /^\s*#/ )
    {
		print OUT "$Lines[$idx]\n";
	    }
	    else
	    {
		print OUT "\n";
	    }
	}

	$idx++;

	last if $idx > $#Lines;
    
    }

    close OUT;


sub removeSpaces
{
  my @inArray = @_;
  my @outArray;
  
  foreach $data ( @inArray )
  {
    push @outArray, $data if $data;
  }

  return @outArray;
}
sub beautyLines
{
  my $lines = shift;
  
  $lines =~ s/([^\!\>\<\+\*\~\|\&\/\-\%\^\/=\'])=([^=\~\'])/$1 = $2/gio;

  $lines =~ s/([^\'])\(([^\'])/$1 \( $2/gio;
  $lines =~ s/([^\'])\)([^\'])/$1 \) $2/gio;
  $lines =~ s/([^\'])\)\s*$/$1 \)/gio;

  $lines =~ s/([^\'])\[([^\'])/$1 \[ $2/gio;
  $lines =~ s/([^\'])\]([^\'])/$1 \]$2/gio;

  $lines =~ s/([^\'])\,([^\'])/$1\, $2/gio;
  $lines =~ s/(\s)\,([^\'])/\, $2/gio;

  $lines =~ s/\(\s+\)/\(\)/gio;
  $lines =~ s/\s+\(\s+/ \( /gio;
  $lines =~ s/\s+\)\s+/ \) /gio;
  $lines =~ s/\)\s*\:/\) \:/gio;
  $lines =~ s/\)\s+\,/\)\,/gio;
  $lines =~ s/\)\s+\;/\)\;/gio;
  $lines =~ s/\[\s+\]/\[\]/gio;
  $lines =~ s/\s+\[\s+/ \[ /gio;
  $lines =~ s/\s+\]\s+/ \] /gio;
  $lines =~ s/\]\s*\:/\] \:/gio;
  $lines =~ s/\]\s+\,/\]\,/gio;
  $lines =~ s/\]\s+\;/\]\;/gio;
  $lines =~ s/\s+\=\s+/ \= /gio;
  $lines =~ s/\s+\:\s+/ \: /gio;
  $lines =~ s/\,\s+/\, /gio;
  $lines =~ s/\s+\;\s*/ \;/gio;

  $lines;
}
