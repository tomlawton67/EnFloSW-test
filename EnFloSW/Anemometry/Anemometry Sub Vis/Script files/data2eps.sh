#! /bin/bash
# To make a symbolic link of this gnuplot shell script data2eps.sh in /usr/local/bin
# Paste the following command into the Terminal:(without the #) then hit return.
# sudo ln -s /Applications/EnFlo\ Software/Anemometry/Anemometry\ Sub\ Vis/Script\ files/data2eps.sh /usr/local/bin/data2eps.sh


rm ./"$1.pdf"

PATH=/sw/bin:/sw/sbin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/teTeX/bin/powerpc-apple-darwin-current:/usr/local/bin:/usr/X11R6/bin:/usr/texbin/
export PATH

/Applications/Gnuplot.app/Contents/Resources/bin/gnuplot gnuplot_script.txt

OUTF=plotwrap.tex
echo "

%This won't typeset correctly with TexShop, as it requires special switches throwing

%\documentclass[12pt]{article}
\documentclass{article}

%Gnuplot produces eps, so don't need graphicx etc
\usepackage{graphics}
\usepackage{amssymb}
\usepackage[pdftex]{graphicx}
\usepackage{epstopdf}
\usepackage{color}


%This tells it that it's just a little picture, not a full page
\pagestyle{empty}


\begin{document}

\include{Plot}

 \end{document} 
" >$OUTF

latex plotwrap.tex
dvips -E -o "$1.eps" plotwrap.dvi


rm ./*.aux
rm ./*.dvi
rm ./terminal.log
# rm ./Plot.eps
# rm ./Plot.tex
# rm ./plotwrap.tex

# rm ./*.tex

epstopdf "$1.eps"

open "$1.pdf"

#rm ./*.eps
