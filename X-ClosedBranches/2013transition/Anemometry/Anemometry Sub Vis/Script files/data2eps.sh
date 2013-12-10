#! /bin/bash
# To make a symbolic link of this gnuplot shell script data2eps.sh in /usr/local/bin
# Paste the following command into the Terminal:(without the #) then hit return.
# sudo ln -s /Applications/EnFlo\ Software/Anemometry/Anemometry\ Sub\ Vis/Script\ files/data2eps.sh /usr/local/bin/data2eps.sh

# Clean up any existing PDF:
rm ./"$1.pdf"

# Make sure we know where to find everything:
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/teTeX/bin/powerpc-apple-darwin-current:/usr/local/bin:/usr/X11R6/bin:/usr/texbin/
export PATH

# Call gnuplot to run the script which generates the graph:
/usr/local/bin/gnuplot gnuplot_script.txt

#--------------------------------------------------------------------------
# Make a "wrapper" file to allow the gnuplot LaTeX to be typeset:
OUTF=plotwrap.tex
echo "

% This won't typeset correctly with TexShop, as it requires special switches throwing

\documentclass{article}

% Gnuplot produces eps, so don't need graphicx etc
\usepackage{graphics}
\usepackage{amssymb}
\usepackage[pdftex]{graphicx}
\usepackage{epstopdf}
\usepackage{color}


% This tells it that it's just a little picture, not a full page
\pagestyle{empty}


\begin{document}

\input{Plot}
% Brings in the LaTeX file made by gnuplot, which itself includes the EPS graphic plot

\end{document} 
" >$OUTF
#--------------------------------------------------------------------------

# Typeset the LaTeX file we made above:
latex plotwrap.tex

# Convert the DVI file from LaTeX to an EPS:
dvips -E -o "$1.eps" plotwrap.dvi

# We /could/ typeset direct to PDF these days, but the EPS is useful for Word. Yeuch.
#--------------------------------------------------------------------------

# Tidy up some auxilliary files:
rm ./*.aux
rm ./*.dvi
rm ./terminal.log

# Best not to tidy these up- useful later:
# rm ./Plot.eps
# rm ./Plot.tex
# rm ./plotwrap.tex
# rm ./*.tex
#--------------------------------------------------------------------------

# Convert the EPS to a PDF:
epstopdf "$1.eps"

open "$1.pdf"

# The EPS with the same name as the PDF can be used in Word, if required. Yeuch.
