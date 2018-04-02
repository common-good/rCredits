CREATE TABLE IF NOT EXISTS `r_nonmembers` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'non-member company record id',
  `company` varchar(60) DEFAULT NULL COMMENT 'company name',
  `potential` int(11) DEFAULT '0' COMMENT 'number of members who shop there',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COMMENT='Local companies we want to recruit';
