#
# Простейший Makefile для создания отчёта
#

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


# UTILS = $(REPORT_DIR)/utils
# PATH_TEX = $(REPORT_DIR)/tex
# PATH_INCLUDE_TEX = $(PATH_TEX)/include
# COV_REPORT = coverage_report.info
# COV_TEXGEN = perl $(UTILS)/lcov-latex/gentex
# #PDF = pdflatex -interaction nonstopmode -halt-on-error
# PDF = pdflatex -shell-escape -interaction nonstopmode
# CFLOW = cflow --level "0= "
# CFLOW2DOT = python $(UTILS)/cflow2dot
# CFLOWFILES = $(shell find $(ROOT)/$(SRC_DIR) -name "*.c")
# FSM2DOT = python $(UTILS)/fsm2dot
# MAKE2DOT = $(UTILS)/makefile2graph/make2graph
# RE2TEX = python $(UTILS)/re2tex
# DOT = dot
# DOXYGEN_LATEX_PATH = $(ROOT)doxygen/latex
# DOXYGEN_FILES = $(PATH_INCLUDE_TEX)/doxygen/config.h.tex $(PATH_INCLUDE_TEX)/doxygen/context.h.tex \
# 				$(PATH_INCLUDE_TEX)/doxygen/logs.h.tex $(PATH_INCLUDE_TEX)/doxygen/maildir.h.tex \
# 				$(PATH_INCLUDE_TEX)/doxygen/util.h.tex $(PATH_INCLUDE_TEX)/doxygen/network.h.tex \
# 				$(PATH_INCLUDE_TEX)/doxygen/smtp.h.tex

# #find src -name "*.c"


# build_report: mkdir_report $(PATH_INCLUDE_TEX)/ut_out.tex $(PATH_INCLUDE_TEX)/smtp_states.pdf \
# 		$(PATH_INCLUDE_TEX)/cflow.pdf $(PATH_INCLUDE_TEX)/make.pdf $(DOXYGEN_FILES)
# 	cd $(PATH_TEX) && $(PDF) main.tex

# #lcov_tex: $(ROOT)/$(COV_REPORT)
# #	$(COV_TEXGEN) $< -o $(PATH_INCLUDE_TEX)/lcov

# $(ROOT)/$(COV_REPORT): 
# 	lcov -t "server testing code coverage" -o $@ -c -d .


# $(PATH_INCLUDE_TEX)/ut_out.tex: $(UNIT_TEST_OUT)
# 	valgrind --log-file=$(PATH_INCLUDE_TEX)/valgrind_out.tex $< > $@

# unit_tests_tex: $(UNIT_TEST_OUT)
# 	valgrind --log-file=$(PATH_INCLUDE_TEX)/valgrind_out.tex $< > $(PATH_INCLUDE_TEX)/unit_test_out.tex

# $(PATH_INCLUDE_TEX)/cflow.dot: $(CFLOWFILES)
# 	$(CFLOW) $^ | $(CFLOW2DOT) > $@ 

# $(PATH_INCLUDE_TEX)/cflow.pdf: $(PATH_INCLUDE_TEX)/cflow.dot
# 	$(DOT) -Tpdf -o $@ $<

# $(PATH_INCLUDE_TEX)/smtp_states.dot: $(SRC_DIR)/smtp/smtp-states.def
# 	 $(FSM2DOT) $(SRC_DIR)/smtp/smtp-states.def > $@

# $(PATH_INCLUDE_TEX)/smtp_states.pdf: $(PATH_INCLUDE_TEX)/smtp_states.dot
# 	$(DOT) -Tpdf -o $@ $< 

# $(PATH_INCLUDE_TEX)/make.dot: Makefile
# 	export LC_ALL=C && make -Bnd all | $(MAKE2DOT) > $(PATH_INCLUDE_TEX)/make.dot

# $(PATH_INCLUDE_TEX)/make.pdf: $(PATH_INCLUDE_TEX)/make.dot
# 	dot -Tpdf -o $@ $<

# #doxygen

# $(DOXYGEN_LATEX_PATH)/config_8h.tex: 
# 	doxygen Doxyfile

# $(DOXYGEN_LATEX_PATH)/context_8h.tex:
# 	doxygen Doxyfile

# $(DOXYGEN_LATEX_PATH)/maildir_8h.tex:
# 	doxygen Doxyfile

# $(DOXYGEN_LATEX_PATH)/logs_8h.tex:
# 	doxygen Doxyfile

# $(DOXYGEN_LATEX_PATH)/util_8h.tex:
# 	doxygen Doxyfile

# $(DOXYGEN_LATEX_PATH)/network_8h.tex:
# 	doxygen Doxyfile

# $(DOXYGEN_LATEX_PATH)/smtp_8h.tex:
# 	doxygen Doxyfile


# $(PATH_INCLUDE_TEX)/doxygen/config.h.tex: $(DOXYGEN_LATEX_PATH)/config_8h.tex
# 	cp $< $@

# $(PATH_INCLUDE_TEX)/doxygen/context.h.tex: $(DOXYGEN_LATEX_PATH)/context_8h.tex
# 	cp $< $@

# $(PATH_INCLUDE_TEX)/doxygen/maildir.h.tex: $(DOXYGEN_LATEX_PATH)/maildir_8h.tex
# 	cp $< $@

# $(PATH_INCLUDE_TEX)/doxygen/logs.h.tex: $(DOXYGEN_LATEX_PATH)/logs_8h.tex
# 	cp $< $@

# $(PATH_INCLUDE_TEX)/doxygen/util.h.tex: $(DOXYGEN_LATEX_PATH)/util_8h.tex
# 	cp $< $@

# $(PATH_INCLUDE_TEX)/doxygen/network.h.tex: $(DOXYGEN_LATEX_PATH)/network_8h.tex
# 	cp $< $@

# $(PATH_INCLUDE_TEX)/doxygen/smtp.h.tex: $(DOXYGEN_LATEX_PATH)/smtp_8h.tex
# 	cp $< $@

# # smtp_pdf: $(PATH_INCLUDE_TEX)/smtp_states.pdf
# # test_make_dot: $(PATH_INCLUDE_TEX)/make.pdf
# # cflow: $(PATH_INCLUDE_TEX)/cflow.pdf
# # doxygen_tex: $(DOXYGEN_FILES)

# mkdir_report:
# 	mkdir -p $(PATH_TEX)/include
# 	mkdir -p $(PATH_TEX)/include/re
# 	mkdir -p $(PATH_TEX)/include/lcov
# 	mkdir -p $(PATH_TEX)/include/doxygen

# #.PHONY: report lcov_tex unit_tests_tex cflow smtp_fsm_pdf $(PATH_INCLUDE_TEX)/make.dot

# .PHONY: regex_to_tex mkdir_report lcov_tex
