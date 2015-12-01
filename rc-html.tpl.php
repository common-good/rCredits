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

$rPath = BASE_URL . R_PATH;
$version = R_VERSION;
$styles = preg_replace('~<style.*</style>~ms', '', $styles); // zap all the drupal styles
$scripts = preg_replace('~misc/jquery.js.*<~', 'sites/all/modules/rcredits/js/jquery.min.js"><', $scripts);
$scripts = preg_replace('~devel_krumo_path.js|user/user.js|drupal.js~ms', 'UNUSED', $scripts);
// something here interferes with croppic's $()
//$scripts = preg_replace('/<script[!<]+<![!<]+<![!<]+jQuery.extend\(Drupal/ms', '', $scripts);
//$scripts = preg_replace('/jQuery.extend.*--/ms', '//--', $scripts);

/**/ echo <<<EOF
<!DOCTYPE html>
<html lang="$language->language" dir="$language->dir"
$rdf_namespaces>

<head profile="$grddl_profile">
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->
  $head
    <meta name="MobileOptimized" content="width" />
    <meta name="HandheldFriendly" content="true" />
    <meta http-equiv="cleartype" content="on" />
  <title>$head_title</title>
  <meta name="description" content="">
  <meta name="author" content="William Spademan -- for Society to Benefit Everyone, Inc.">
  <link rel="stylesheet" href="$rPath/css/bootstrap.min.css" />
  <link rel="stylesheet" href="$rPath/css/jquery-ui.css">
  <link rel="stylesheet" href="$rPath/css/ladda-themeless.min.css">
  $styles
  <link rel="stylesheet" href="$rPath/css/rcredits.css?$version" type="text/css" />
  <link rel="stylesheet" href="$rPath/css/rweb.css?$version" type="text/css" />
  $scripts
</head>

<body class="$classes" $attributes>
  $page_top
  $page
  $page_bottom
</body>
</html>
EOF;
