CREATE TABLE IF NOT EXISTS `r_countries` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Country Id',
  `name` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'Country Name',
  `iso_code` char(2) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'ISO Code',
  `country_code` varchar(4) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'National prefix to be used when dialing TO this country.',
  `address_format_id` int(10) unsigned DEFAULT NULL COMMENT 'Foreign key to civicrm_address_format.id.',
  `idd_prefix` varchar(4) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'International direct dialing prefix from within the country TO another country',
  `ndd_prefix` varchar(4) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'Access prefix to call within a country to a different area',
  `region_id` int(10) unsigned NOT NULL COMMENT 'Foreign key to civicrm_worldregion.id.',
  `is_province_abbreviated` tinyint(4) DEFAULT '0' COMMENT 'Should state/province be displayed as abbreviation?',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_iso_code` (`name`,`iso_code`),
  KEY `address_format_id` (`address_format_id`),
  KEY `region_id` (`region_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1247 ;
