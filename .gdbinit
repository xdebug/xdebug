define __pxdllist
	set $l = (xdebug_llist*)$arg0
	set $h = $l->head

	print *(xdebug_hash_element*)((*(xdebug_llist_element *) $h).ptr)
	while $h->next
		print *(xdebug_hash_element*)((*(xdebug_llist_element *) $h).ptr)
	end
end

define pxdhash
	set $ht = (xdebug_hash*)$arg0
	set $n = $ht->slots
	set $i = 0
	while $i < $n
		if $ht->table[$i]->size != 0
			printf "Slot %d →\n", $i
			__pxdllist $ht->table[$i]
			printf "\n"
		end
		set $i = $i + 1
	end
end

define pxdset
	set $s = (xdebug_set*) $arg0
	printf "Set: size: %d: ", $s->size
	set $i = 0
	while $i < $s->size
		set $byte = &($s->setinfo[$i / 8])
		set $bit = $i % 8
		if (*$byte & (1 << $bit))
			printf "{%d} ", $i
		end
		set $i = $i + 1
	end
	printf "\n"
end
