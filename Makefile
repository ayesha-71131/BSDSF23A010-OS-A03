# ======================================================
#   Outer Makefile (project root)
#   Just forwards commands to inner Makefile
# ======================================================

.PHONY: all clean run

all:
	@$(MAKE) -C Makefilee all

run:
	@$(MAKE) -C Makefilee run

clean:
	@$(MAKE) -C Makefilee clean
