DIRS = $(shell find -maxdepth 1 -type d ! -name '.git' ! -name '.' ! -name 'tmp' -printf '%f\n' | sort)

all:
	@for dir in $(DIRS); do \
      cd $${dir}; \
      ${MAKE} all ; \
      cd .. ; \
    done ; \

# clean *.exe under Linux
clean:
	@for dir in $(DIRS); do \
	  cd $${dir}; \
	  ${MAKE} clean; \
	  find -name '*.exe' -printf "delete %P\n" -delete; \
	  cd .. ; \
	done; \
