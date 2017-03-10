<?php

/**
 * @file
 * Default theme implementation to display the basic html structure of a single Drupal page.
 * called from the stub html.tpl.php in the theme's templates folder.
 *
 * Variables:
 * - $css: An array of CSS files for the current page.
 * - $language: (object) The language the site is being displayed in.
 *   $language->language contains its textual representation.
 *   $language->dir contains the language direction. It will either be 'ltr' or 'rtl'.
 * - $rdf_namespaces: All the RDF namespace prefixes used in the HTML document.
 * - $grddl_profile: A GRDDL profile allowing agents to extract the RDF data.
 * - $head_title: A modified version of the page title, for use in the TITLE
 *   tag.
 * - $head_title_array: (array) An associative array containing the string parts
 *   that were used to generate the $head_title variable, already prepared to be
 *   output as TITLE tag. The key/value pairs may contain one or more of the
 *   following, depending on conditions:
 *   - title: The title of the current page, if any.
 *   - name: The name of the site.
 *   - slogan: The slogan of the site, if any, and if there is no title.
 * - $head: Markup for the HEAD section (including meta tags, keyword tags, and
 *   so on).
 * - $styles: Style tags necessary to import all CSS files for the page.
 * - $scripts: Script tags necessary to load the JavaScript files and settings
 *   for the page.
 * - $page_top: Initial markup from any modules that have altered the
 *   page. This variable should always be output first, before all other dynamic
 *   content.
 * - $page: The rendered page content.
 * - $page_bottom: Final closing markup from any modules that have altered the
 *   page. This variable should always be output last, after all other dynamic
 *   content.
 * - $classes String of classes that can be used to style contextually through
 *   CSS.
 *
 * @see template_preprocess()
 * @see template_preprocess_html()
 * @see template_process()
 */


$unused = join('|', ['sites/all/modules/devel/devel_krumo_path.js', 'misc/drupal.js']); // also modules/user/user.js?
global $rUrl, $base_url;
$version = isPRODUCTION ? R_VERSION : time();
$styles = preg_replace('~<style.*</style>~ms', '', $styles); // zap all the drupal styles
//$scripts = preg_replace('~misc/jquery.js.*<~', "sites/all/modules/rcredits/js/jquery.min.js?$version\"><", $scripts);
$scripts = preg_replace("~$base_url/misc/jquery.js.*<~", "$rUrl/js/jquery.min.js?$version\"><", $scripts);
//$scripts = preg_replace('~misc/jquery.js.*<~', "../js/jquery.min.js?$version\"><", $scripts);
$scripts = preg_replace('~<script type="text/javascript" src="' . BASE_URL . "/($unused)" . '\?[a-z0-9]+"></script>~ms', '', $scripts);
// something here interferes with croppic's $()
//$scripts = preg_replace('/<script[!<]+<![!<]+<![!<]+jQuery.extend\(Drupal/ms', '', $scripts);
//$scripts = preg_replace('/jQuery.extend.*--/ms', '//--', $scripts);
$page_bottom = str_replace('?' . \variable_get('css_js_query_string', 'un like ly'), "?$version", $page_bottom);
$favicon = <<<EOF
  <link rel="apple-touch-icon" sizes="180x180" href="$rUrl/images/favicons/apple-touch-icon.png">
  <link rel="icon" type="image/png" href="$rUrl/images/favicons/favicon-32x32.png" sizes="32x32">
  <link rel="icon" type="image/png" href="$rUrl/images/favicons/favicon-16x16.png" sizes="16x16">
  <link rel="manifest" href="$rUrl/images/favicons/manifest.json">
  <link rel="mask-icon" href="$rUrl/images/favicons/safari-pinned-tab.svg" color="#5bbad5">
  <link rel="shortcut icon" href="$rUrl/images/favicons/favicon.ico">
  <meta name="msapplication-config" content="$rUrl/images/favicons/browserconfig.xml">
  <meta name="theme-color" content="#ffffff">
EOF;
//  : '  <link rel="shortcut icon" href="' . $rUrl . '/images/icons/rFavicon.ico" type="image/vnd.microsoft.icon" />';

/**/ echo <<<EOF
<!DOCTYPE html>
<html lang="$language->language" dir="$language->dir"
$rdf_namespaces>

<head profile="$grddl_profile">
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <!-- The above 3 meta tags *must* come first in the head -->

  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

$favicon
  <meta name="MobileOptimized" content="width" />
  <meta name="HandheldFriendly" content="true" />
  <meta name="apple-mobile-web-app-capable" content="no"><!-- (not yet) -->
  <meta http-equiv="cleartype" content="on" />
  <title>$head_title</title>
  <meta name="description" content="">
  <meta name="author" content="William Spademan -- for Society to Benefit Everyone, Inc.">
  <link rel="stylesheet" href="$rUrl/css/bootstrap.min.css?$version" />
  <link rel="stylesheet" href="$rUrl/css/jquery-ui.css?$version">
  <link rel="stylesheet" href="$rUrl/css/ladda-themeless.min.css?$version">
  $styles
  <link rel="stylesheet" href="$rUrl/css/rcredits.css?$version" type="text/css" />
  <link rel="stylesheet" href="$rUrl/css/rweb.css?$version" type="text/css" />
  $scripts
</head>

<body class="$classes" $attributes>
  $page_top
  $page
  $page_bottom
</body>
</html>
EOF;
