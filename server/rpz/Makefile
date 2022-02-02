
PDF=rpz.pdf

# Используемые программы
PDFLATEX = pdflatex -interaction=nonstopmode

TEX=tex
INCLUDES = $(TEX)/includes

.PHONY: clean
all : $(PDF)
rpz : $(PDF)


$(INCLUDES)/%.pdf : $(INCLUDES)/%.tex
	$(PDFLATEX) -output-directory=$(INCLUDES) $<

# Основная цель сборки 
$(PDF) : $(TEX)/rpz.tex $(TEX)/header.tex
	cd $(TEX) && $(PDFLATEX) rpz.tex && $(PDFLATEX) rpz.tex && cp $(PDF) ..

# Удаление ВСЕХ временных файлов
clean:
	find $(TEX)/ -regextype posix-egrep -type f ! -regex ".*\.(sty|tex|clo|cls|bib|bst|gitignore)" -exec $(RM) {} \; ;


