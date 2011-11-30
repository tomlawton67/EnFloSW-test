
del %1.pdf


echo calling gnuplot
wgnuplot gnuplot_script.txt
echo finished


echo \documentclass{article} >plotwrap.tex
echo \usepackage{graphics} >>plotwrap.tex
echo \usepackage{amssymb} >>plotwrap.tex
echo \usepackage[pdftex]{graphicx} >>plotwrap.tex
echo \usepackage{epstopdf} >>plotwrap.tex
echo \usepackage{color} >>plotwrap.tex
echo \pagestyle{empty} >>plotwrap.tex
echo \begin{document} >>plotwrap.tex
echo \include{Plot} >>plotwrap.tex
echo \end{document} >>plotwrap.tex 


latex plotwrap.tex
dvips -E -o %1.eps plotwrap.dvi


del *.aux
del *.dvi
del terminal.log
REM del Plot.eps
REM del Plot.tex
REM del plotwrap.tex

REM del *.tex

epstopdf %1.eps

start %1.pdf

REM del *.eps
