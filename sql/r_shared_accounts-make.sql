CREATE TABLE IF NOT EXISTS `r_shared_accounts` (
  `shared_uid` int(11) NOT NULL DEFAULT '0' COMMENT 'uid of the account being shared',
  `shared_with` int(11) NOT NULL DEFAULT '0' COMMENT 'uid of a user that has access to that account',
  `permission` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'what type of permission the shared_with user has on the shared_uid account',
  PRIMARY KEY (`shared_uid`,`shared_with`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Which accounts are with which users';
