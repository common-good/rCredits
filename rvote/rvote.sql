INSERT INTO `r_questions` (`id`, `ctty`, `text`, `detail`, `linkideas`, `linkproposals`, `type`, `budget`, `minveto`, `optorder`, `modified`, `created`) VALUES
(1, -410044001, 'What temperature do you prefer indoors?', 'We will adjust the thermostat accordingly.', 'http://commongoodbank.com/forum', 'http://commongoodbank.com/forum', 'M', 0, 0, 'N', '2010-04-30 11:07:30', '2010-03-18 09:06:11'),
(2, 1, 'What shall we invest in this year?', 'Here are some options to choose from.', 'http://commongoodbank.com/forum', 'http://commongoodbank.com/forum', 'B', 100000, 0, 'N', '2010-05-14 11:50:16', '2010-03-18 09:06:11');

INSERT INTO `r_options` (`id`, `text`, `detail`, `question`, `displayorder`, `averagegrade`, `vetoes`, `modified`, `created`) VALUES
(1, '68°F', 'The cool choice.', 1, 0, 0, 0, '2010-03-18 07:48:33', '0000-00-00 00:00:00'),
(2, '70°F', 'The traditional value.', 1, 0, 0, 0, '2010-03-18 07:48:33', '0000-00-00 00:00:00'),
(3, '76°F', 'The hot choice.', 1, 0, 0, 0, '2010-03-18 07:48:53', '0000-00-00 00:00:00'),
(4, 'Home mortgages', 'More people get a home.', 2, 0, 0, 0, '2010-04-06 16:56:06', '2010-03-18 16:12:51'),
(5, 'Agricultural infrastructure', 'More people eat. If you''re interested in getting fresh food directly from a local farm, you might consider purchasing a farm share. We''re maintaining a list of farms with available CSA (Community Supported Agriculture) shares to make it easy to find a good fit near you. \r\nPVGrows Infrastructure Finance Project for Food System Enterprises\r\n\r\nPioneer Valley Grows is offering new loan options and creative financing opportunities for enterprises that help bring more local food to markets and address food system infrastructure gaps in the Pioneer Valley. ', 2, 0, 0, 0, '2010-04-06 16:56:06', '2010-03-18 16:12:51'),
(6, 'Energy generation', 'Plenty of power.', 2, 0, 0, 0, '2010-04-06 16:56:06', '2010-03-18 16:14:02');