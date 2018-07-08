USE new_cg;
UPDATE variable SET value='b:1;' WHERE name='up';
TRUNCATE cache;
TRUNCATE cache_bootstrap;
TRUNCATE cache_form;
TRUNCATE cache_menu;
