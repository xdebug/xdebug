all-optional: debugclient/main.c debugclient/usefulstuff.c
	@gcc -Wall -g -o debugclient/debugclient debugclient/main.c debugclient/usefulstuff.c
	@cp debugclient/debugclient $(prefix)/bin/

install-optional:
	@echo "Installing debugclient into '$(prefix)/bin/.'"
	@cp debugclient/debugclient $(prefix)/bin/
