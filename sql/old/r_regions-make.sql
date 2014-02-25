CREATE TABLE IF NOT EXISTS `r_regions` (
  `region` char(3) COLLATE utf8_unicode_ci NOT NULL COMMENT 'credit region id',
  `state` char(2) COLLATE utf8_unicode_ci NOT NULL COMMENT 'state or province',
  `zips` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`region`),
  KEY `state` (`state`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;