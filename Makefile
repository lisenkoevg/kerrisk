DIRS = $(shell find -maxdepth 1 -type d ! -name '.git' ! -name '.' ! -name 'tmp' -printf '%f\n')

# Force rebuild if build from different platform/machine (uname changed)
uname_current=$(shell uname -n)
uname_built=$(shell cat uname 2> /dev/null)
ifeq ($(uname_current), $(uname_built))
  rebuild :=
else
  rebuild := -B
endif

all:
	@for dir in $(DIRS); do                             \
      err= ;                                            \
      cd $${dir};                                       \
      ${MAKE} ${rebuild} all || err=1 ;              \
      cd .. ;                                           \
    done ;                                              \
    test -z "$$err" && echo $(uname_current) > uname

clean:
	@for dir in $(DIRS); do              \
      cd $${dir}; ${MAKE} clean; cd .. ; \
    done; \
    rm -f uname
