-- phpMyAdmin SQL Dump
-- version 3.4.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Aug 27, 2012 at 07:58 AM
-- Server version: 5.5.16
-- PHP Version: 5.3.8

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `devcore`
--

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `uid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Primary Key: Unique user ID.',
  `name` varchar(60) NOT NULL DEFAULT '' COMMENT 'Unique user name.',
  `pass` varchar(128) NOT NULL DEFAULT '' COMMENT 'User’s password (hashed).',
  `mail` varchar(254) DEFAULT '' COMMENT 'User’s e-mail address.',
  `theme` varchar(255) NOT NULL DEFAULT '' COMMENT 'User’s default theme.',
  `signature` varchar(255) NOT NULL DEFAULT '' COMMENT 'User’s signature.',
  `signature_format` varchar(255) DEFAULT NULL COMMENT 'The filter_format.format of the signature.',
  `created` int(11) NOT NULL DEFAULT '0' COMMENT 'Timestamp for when user was created.',
  `access` int(11) NOT NULL DEFAULT '0' COMMENT 'Timestamp for previous time user accessed the site.',
  `login` int(11) NOT NULL DEFAULT '0' COMMENT 'Timestamp for user’s last login.',
  `status` tinyint(4) NOT NULL DEFAULT '0' COMMENT 'Whether the user is active(1) or blocked(0).',
  `timezone` varchar(32) DEFAULT NULL COMMENT 'User’s time zone.',
  `language` varchar(12) NOT NULL DEFAULT '' COMMENT 'User’s default language.',
  `picture` int(11) NOT NULL DEFAULT '0' COMMENT 'Foreign key: file_managed.fid of user’s picture.',
  `init` varchar(254) DEFAULT '' COMMENT 'E-mail address used for initial account creation.',
  `data` longblob COMMENT 'A serialized array of name value pairs that are related to the user. Any form values posted during user edit are stored and are loaded into the $user object during user_load(). Use of this field is discouraged and it will likely disappear in a future...',
  `account_type` tinyint(4) DEFAULT NULL COMMENT 'personal, commercial, or non-profit',
  `federalId` varchar(9) DEFAULT NULL COMMENT 'social security number or employer id number, for reporting',
  `dob` int(11) DEFAULT NULL COMMENT 'date of birth, founding, or incorporation',
  `fullName` varchar(60) DEFAULT NULL COMMENT 'full name of the individual or entity',
  `short_name` varchar(60) DEFAULT NULL COMMENT 'alphanumeric identifier with no spaces (internal)',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone (no country code, no punctuation)',
  `fax` varchar(255) DEFAULT NULL COMMENT 'fax number (no country code, no punctuation)',
  `address` varchar(60) DEFAULT NULL COMMENT 'postal street address',
  `city` varchar(60) DEFAULT NULL COMMENT 'municipality',
  `state` varchar(60) DEFAULT NULL COMMENT 'full state/province name',
  `postalCode` varchar(20) DEFAULT NULL COMMENT 'contact postal code (no punctuation)',
  `country` varchar(60) DEFAULT NULL COMMENT 'full country name',
  `website` varchar(255) DEFAULT NULL COMMENT 'primary website',
  `categories` mediumtext COMMENT 'business categories',
  `notes` mediumtext COMMENT 'miscellaneous notes about the user or the account',
  `credit_id` varchar(9) DEFAULT NULL COMMENT 'unique credit account identifier (8 characters for individuals)',
  `rebate` decimal(4,3) DEFAULT '5.000' COMMENT 'current rebate percentage (sales bonus is double)',
  `demand` decimal(9,2) DEFAULT '0.00' COMMENT 'waiting to buy this much credit',
  `min_balance` decimal(9,2) DEFAULT '-10000.00' COMMENT 'minimum balance (normally zero or less)',
  `lost_card` tinyint(3) unsigned DEFAULT '0' COMMENT 'user lost the account id card',
  `bank_account_number` varchar(34) DEFAULT NULL COMMENT 'official currency international bank account number',
  `bank_account_name` varchar(60) DEFAULT NULL COMMENT 'name on US$ bank account -- usually the same as name',
  `bank_account_verified` tinyint(3) unsigned DEFAULT '0' COMMENT 'user has proven ownership of account',
  PRIMARY KEY (`uid`),
  UNIQUE KEY `name` (`name`),
  KEY `access` (`access`),
  KEY `created` (`created`),
  KEY `mail` (`mail`),
  KEY `picture` (`picture`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Stores user data.';

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`uid`, `name`, `pass`, `mail`, `theme`, `signature`, `signature_format`, `created`, `access`, `login`, `status`, `timezone`, `language`, `picture`, `init`, `data`, `account_type`, `federalId`, `dob`, `fullName`, `short_name`, `phone`, `fax`, `address`, `city`, `state`, `postalCode`, `country`, `website`, `categories`, `notes`, `credit_id`, `rebate`, `demand`, `min_balance`, `lost_card`, `bank_account_number`, `bank_account_name`, `bank_account_verified`) VALUES
(0, '', '', '', '', '', NULL, 0, 0, 0, 0, NULL, '', 0, '', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 5.000, 0.00, -10000.00, 0, NULL, NULL, 0),
(1, 'admin', '$S$D.yi/fTvY3CjOqPX2oyW1OgEIq6wj4jxMKOlAKXgevKPEtAZMdYi', 'wspademan@gmail.com', '', '', NULL, 1335463095, 1346045837, 1346028502, 1, 'America/New_York', '', 0, 'wspademan@gmail.com', 0x623a303b, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, '01330', NULL, NULL, NULL, NULL, NULL, 5.000, 0.00, -10000.00, 0, NULL, NULL, 0),
(129, 'Community', '$S$Du27exhBr0sQ.GwFTk111lzxFwgChiT4Ve3HGLExFt/8EXZAbDpO', '', '', '', NULL, 1339798842, 0, 0, 1, NULL, '', 0, '', 0x623a303b, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, '01%', NULL, NULL, 'N;', NULL, 'community', 0.000, 25000.00, -10000.00, 0, NULL, NULL, 0),
(130, 'uten', '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae', 'uten@ex.com', '', '', NULL, 0, 1346046534, 1346046534, 1, NULL, '', 0, '', NULL, NULL, NULL, NULL, 'U Ten', 'uten', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'Z9AAAAAA', 5.000, 0.00, -10000.00, 0, NULL, NULL, 0),
(131, 'btwo', '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae', 'btwo@ex.com', '', '', NULL, 0, 0, 0, 1, NULL, '', 0, '', NULL, NULL, NULL, NULL, 'B Two', 'btwo', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'Z9AAAAAB', 5.000, 0.00, -10000.00, 0, NULL, NULL, 0),
(132, 'cstore', '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae', 'cstore@ex.com', '', '', NULL, 0, 0, 0, 1, NULL, '', 0, '', NULL, NULL, NULL, NULL, 'Corner Store', 'cornerstore', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'Z9AAAAAC', 5.000, 0.00, -10000.00, 0, NULL, NULL, 0);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
