CREATE TABLE IF NOT EXISTS `r_credit_regions` (
  `region` char(2) COLLATE utf8_unicode_ci NOT NULL COMMENT 'state or province',
  `credit_region` char(2) COLLATE utf8_unicode_ci NOT NULL COMMENT 'credit region id',
  PRIMARY KEY (`region`),
  KEY `credit_region` (`credit_region`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;