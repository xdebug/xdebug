all-optional: debugclient/main.c usefulstuff.c
	@gcc -Wall -g -o debugclient/debugclient debugclient/main.c usefulstuff.c

install-optional:
	@echo "Installing debugclient into '$(prefix)/bin/.'"
	@cp debugclient/debugclient $(prefix)/bin/
