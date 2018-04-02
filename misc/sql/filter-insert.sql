INSERT INTO `filter` (`format`, `module`, `name`, `weight`, `status`, `settings`) VALUES
('filtered_html', 'filter', 'filter_autop', 2, 1, 'a:0:{}'),
('filtered_html', 'filter', 'filter_html', 1, 1, 'a:3:{s:12:"allowed_html";s:74:"<a> <em> <strong> <cite> <blockquote> <code> <ul> <ol> <li> <dl> <dt> <dd>";s:16:"filter_html_help";i:1;s:20:"filter_html_nofollow";i:0;}'),
('filtered_html', 'filter', 'filter_htmlcorrector', 10, 1, 'a:0:{}'),
('filtered_html', 'filter', 'filter_html_escape', -10, 0, 'a:0:{}'),
('plain_text', 'filter', 'filter_autop', 2, 1, 'a:0:{}'),
('plain_text', 'filter', 'filter_html', -10, 0, 'a:3:{s:12:"allowed_html";s:74:"<a> <em> <strong> <cite> <blockquote> <code> <ul> <ol> <li> <dl> <dt> <dd>";s:16:"filter_html_help";i:1;s:20:"filter_html_nofollow";i:0;}'),
('plain_text', 'filter', 'filter_htmlcorrector', 10, 0, 'a:0:{}'),
('plain_text', 'filter', 'filter_html_escape', 0, 1, 'a:0:{}'),
('plain_text', 'filter', 'filter_url', 1, 1, 'a:1:{s:17:"filter_url_length";i:72;}');