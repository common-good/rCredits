CREATE TABLE IF NOT EXISTS `r_credit_tails` (
  `community` char(4) COLLATE utf8_unicode_ci NOT NULL COMMENT '4-character community identifier',
  `tail` int(11) NOT NULL DEFAULT '0' COMMENT 'maximum credit id tail used so far',
  PRIMARY KEY (`community`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;