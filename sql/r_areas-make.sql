CREATE TABLE IF NOT EXISTS `r_areas` (
  `area_code` char(3) DEFAULT NULL COMMENT 'telephone area code',
  `region` varchar(24) DEFAULT NULL COMMENT 'state, province, or territory',
  PRIMARY KEY (`area_code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;