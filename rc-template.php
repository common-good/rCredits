<?php
use rCredits as r;
use rCredits\Util as u;
use rCredits\Web as w;

/**
 * @file
 * rCredits template functions, called from the rCredits theme's template.php stub
 */
 
function rcredits_preprocess_html(&$variables) {
  // Add variables for path to theme.
  $variables['base_path'] = base_path();
  $variables['path_to_rcredits_theme'] = drupal_get_path('theme', 'rcredits');
  
  // Add body classes if certain regions have content.
  if (!empty($variables['page']['accounts'])) { // CGF addition
    $variables['classes_array'][] = 'accounts';
  }
  if (!empty($variables['page']['featured'])) {
    $variables['classes_array'][] = 'featured';
  }

  if (!empty($variables['page']['triptych_first'])
    || !empty($variables['page']['triptych_middle'])
    || !empty($variables['page']['triptych_last'])) {
    $variables['classes_array'][] = 'triptych';
  }

  if (!empty($variables['page']['footer_firstcolumn'])
    || !empty($variables['page']['footer_secondcolumn'])
    || !empty($variables['page']['footer_thirdcolumn'])
    || !empty($variables['page']['footer_fourthcolumn'])) {
    $variables['classes_array'][] = 'footer-columns';
  }
}

/**
 * Override or insert variables into the page template for HTML output.
 */
function rcredits_process_html(&$variables) {
  // Hook into color.module.
  if (module_exists('color')) {
    _color_html_alter($variables);
  }
}

/**
 * Override or insert variables into the page template.
 */
function rcredits_process_page(&$variables) {
  // Hook into color.module.
  if (module_exists('color')) {
    _color_page_alter($variables);
  }
  // Always print the site name and slogan, but if they are toggled off, we'll
  // just hide them visually.
  $variables['hide_site_name']   = theme_get_setting('toggle_name') ? FALSE : TRUE;
  $variables['hide_site_slogan'] = theme_get_setting('toggle_slogan') ? FALSE : TRUE;
  if ($variables['hide_site_name']) {
    // If toggle_name is FALSE, the site_name will be empty, so we rebuild it.
    $variables['site_name'] = filter_xss_admin(variable_get('site_name', 'Drupal'));
  }
  if ($variables['hide_site_slogan']) {
    // If toggle_site_slogan is FALSE, the site_slogan will be empty, so we rebuild it.
    $variables['site_slogan'] = filter_xss_admin(variable_get('site_slogan', ''));
  }
  // Since the title and the shortcut link are both block level elements,
  // positioning them next to each other is much simpler with a wrapper div.
  if (!empty($variables['title_suffix']['add_or_remove_shortcut']) && $variables['title']) {
    // Add a wrapper div using the title_prefix and title_suffix render elements.
    $variables['title_prefix']['shortcut_wrapper'] = array(
      '#markup' => '<div class="shortcut-wrapper clearfix">',
      '#weight' => 100,
    );
    $variables['title_suffix']['shortcut_wrapper'] = array(
      '#markup' => '</div>',
      '#weight' => -99,
    );
    // Make sure the shortcut link is the first item in title_suffix.
    $variables['title_suffix']['add_or_remove_shortcut']['#weight'] = -100;
  }
}

/**
 * Implements hook_preprocess_maintenance_page().
 */
function rcredits_preprocess_maintenance_page(&$variables) {
  // By default, site_name is set to Drupal if no db connection is available
  // or during site installation. Setting site_name to an empty string makes
  // the site and update pages look cleaner.
  // @see template_preprocess_maintenance_page
  if (!$variables['db_is_active']) {
    $variables['site_name'] = '';
  }
  drupal_add_css(drupal_get_path('theme', 'rcredits') . '/css/maintenance-page.css');
}

/**
 * Override or insert variables into the maintenance page template.
 */
function rcredits_process_maintenance_page(&$variables) {
  // Always print the site name and slogan, but if they are toggled off, we'll
  // just hide them visually.
  $variables['hide_site_name']   = theme_get_setting('toggle_name') ? FALSE : TRUE;
  $variables['hide_site_slogan'] = theme_get_setting('toggle_slogan') ? FALSE : TRUE;
  if ($variables['hide_site_name']) {
    // If toggle_name is FALSE, the site_name will be empty, so we rebuild it.
    $variables['site_name'] = filter_xss_admin(variable_get('site_name', 'Drupal'));
  }
  if ($variables['hide_site_slogan']) {
    // If toggle_site_slogan is FALSE, the site_slogan will be empty, so we rebuild it.
    $variables['site_slogan'] = filter_xss_admin(variable_get('site_slogan', ''));
  }
}

/**
 * Override or insert variables into the node template.
 */
function rcredits_preprocess_node(&$variables) {
  if ($variables['view_mode'] == 'full' && node_is_page($variables['node'])) {
    $variables['classes_array'][] = 'node-full';
  }
}

/**
 * Override or insert variables into the block template.
 */
function rcredits_preprocess_block(&$variables) {
  // In the header region visually hide block titles.
  if ($variables['block']->region == 'header') {
    $variables['title_attributes_array']['class'][] = 'element-invisible';
  }
}

/**
 * Implements theme_menu_tree().
 */
function rcredits_menu_tree($variables) {
  return '<ul class="menu clearfix">' . $variables['tree'] . '</ul>';
}

/**
 * Implements theme_field__field_type().
 */
function rcredits_field__taxonomy_term_reference($variables) {
  $output = '';

  // Render the label, if it's not hidden.
  if (!$variables['label_hidden']) {
    $output .= '<h3 class="field-label">' . $variables['label'] . ': </h3>';
  }

  // Render the items.
  $output .= ($variables['element']['#label_display'] == 'inline') ? '<ul class="links inline">' : '<ul class="links">';
  foreach ($variables['items'] as $delta => $item) {
    $output .= '<li class="taxonomy-term-reference-' . $delta . '"' . $variables['item_attributes'][$delta] . '>' . drupal_render($item) . '</li>';
  }
  $output .= '</ul>';

  // Render the top-level DIV.
  $output = '<div class="' . $variables['classes'] . (!in_array('clearfix', $variables['classes_array']) ? ' clearfix' : '') . '"' . $variables['attributes'] .'>' . $output . '</div>';

  return $output;
}

function rcredits_links($variables) {
//  global $called; if (@$called) {die('you must disable secondary menu in theme settings');} else $called = TRUE;
  global $rUrl, $base_url, $rMenu;
  $links = $variables['links'];
  $attributes = $variables['attributes'];
  $attributes['class'][] = 'nav navbar-nav'; // cgf
  global $language_url;
  $menuLinks = '';
  $num_links = count($links);
  $i = 1;

  foreach ($links as $key => $link) {
    $class = 'menu-' . preg_replace('/[^a-z\-]+/', '', strtolower(str_replace(' ', '-', $link['title'])));
    $class = array($class); // cgf (with previous line) -- NOTE: this bizarrely has no effect on submenus

    if ($i == 1) $class[] = 'first';
    if ($i == $num_links) $class[] = 'last';
    if (isset($link['href']) && ($link['href'] == $_GET['q'] || ($link['href'] == '<front>' && drupal_is_front_page())) && (empty($link['language']) || $link['language']->language == $language_url->language)) {
      $class[] = 'active';
    }
    $menuLinks .= '<li' . drupal_attributes(array('class' => $class)) . '>';

    if (isset($link['href'])) {
      // Pass in $link as $options, they share the same keys.
      $menuLinks .= l($link['title'], $link['href'], $link);
    }
    elseif (!empty($link['title'])) {
      // Some links are actually not links, but we wrap these in <span> for adding title and class attributes.
      if (empty($link['html'])) {
        $link['title'] = check_plain($link['title']);
      }
      $span_attributes = '';
      if (isset($link['attributes'])) {
        $span_attributes = drupal_attributes($link['attributes']);
      }
      $menuLinks .= '<span' . $span_attributes . '>' . $link['title'] . '</span>';
    }

    $i++;
    $menuLinks .= "</li>\n";
  }

  if ($mya = r\acct()) {
    $acctName = "$mya->fullName ($mya->mainQid)";
    $co = $mya->co ? ' co' : '';
  } else $acctName = $co = '';

  $breadcrumb = rcredits_breadcrumb(['breadcrumb' => \drupal_get_breadcrumb()]);
  
  return <<<EOF
<div class="nav2">
  <div class="nav2-inner">
    <div id="extras" class="">
      <button type="button" class="accounts-toggle photo$co" data-toggle="collapse" data-target="#edit-acct-account" aria-expanded="false" aria-controls="edit-acct-account">
        <a title="$acctName" data-toggle="popover" class="nospin"><img src="$base_url/account/icon"/></a>
      </button>
    </div>
  </div>
</div>

<nav class="navbar navbar-inverse navbar-fixed-top">
  <div class="container">
    <ol class="breadcrumb">
      $breadcrumb
    </ol>
  
    <div class="navbar-header">
      <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">
        <span class="sr-only">Toggle navigation</span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
      <a class="navbar-brand" href="#"><div><img src="$rUrl/images/rlogo-circle80.png" /></div></a>
    </div>
    <div id="navbar" class="collapse navbar-collapse">
      <ul class="nav navbar-nav">
$menuLinks
      </ul>
    </div><!--/.navbar-collapse -->
  </div>
</nav>
EOF;
}

/**
 * Returns HTML for a form element.
 *
 * Each form element is wrapped in a DIV container having the following CSS
 * classes:
 * - form-item: Generic for all form elements.
 * - form-type-#type: The internal element #type.
 * - form-item-#name: The internal form element #name (usually derived from the
 *   $form structure and set via form_builder()).
 * - form-disabled: Only set if the form element is #disabled.
 *
 * In addition to the element itself, the DIV contains a label for the element
 * based on the optional #title_display property, and an optional #description.
 *
 * The optional #title_display property can have these values:
 * - before: The label is output before the element. This is the default.
 *   The label includes the #title and the required marker, if #required.
 * - after: The label is output after the element. For example, this is used
 *   for radio and checkbox #type elements as set in system_element_info().
 *   If the #title is empty but the field is #required, the label will
 *   contain only the required marker.
 * - invisible: Labels are critical for screen readers to enable them to
 *   properly navigate through forms but can be visually distracting. This
 *   property hides the label for everyone except screen readers.
 * - attribute: Set the title attribute on the element to create a tooltip
 *   but output no label element. This is supported only for checkboxes
 *   and radios in form_pre_render_conditional_form_element(). It is used
 *   where a visual label is not needed, such as a table of checkboxes where
 *   the row and column provide the context. The tooltip will include the
 *   title and required marker.
 *
 * If the #title property is not set, then the label and any required marker
 * will not be output, regardless of the #title_display or #required values.
 * This can be useful in cases such as the password_confirm element, which
 * creates children elements that have their own labels and required markers,
 * but the parent element should have neither. Use this carefully because a
 * field without an associated label can cause accessibility challenges.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #title_display, #description, #id, #required,
 *     #children, #type, #name.
 *
 * @ingroup themeable
 */
function rcredits_form_element($variables) {
  extract(rcElement($variables, 'name tabled type markup description parents children required title title_display id field_prefix field_suffix inline class box'));
//debug(compact('tabled', 'variables'));
  $for = @$name ?: strtr(@$parents[0], [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']);
  if ($for == 'title') $children = "<h3>$children</h3>";
  $children = @$field_prefix . $children . @$field_suffix;

  u\setDft($title_display, 'before');
//  $boxy = in_array($type, ['radio', 'checkbox']) ? ($type . (@$inline ? '-inline' : '')) : '';
  $boxy = in_array($type, ['radio', 'checkbox']) ? $type : '';

  $class[] = $boxy ? 'option' : 'control-label col-sm-offset-1 col-xs-2';
  if ($title_display == 'invisible') $class[] = 'sr-only';
  $label = u\tag('label', $boxy ? $children . $title : $title, compact('for', 'class'));
    
  if (@$tabled) {
    if (is_array(@$description)) $description = @$description[1]; // ignore hint here (see rcredits_textfield)
    $description = @$description ? "\n      <div class=\"help-block\">$description</div>" : '';
    $lineup = is_null(@$markup) ? '' : ' lineup';
    $core = <<<EOF
    $label
    <div class="control-data col-xs-10 col-sm-6$lineup">
      $children$description
    </div>
EOF;
  } else $core = $boxy ? $label : $label . $children;

  $id = @$markup ? " id=\"edit-$for\"" : '';
  $outerClass = "form-group form-item-$for";
  $outerClass .= ' ' . ($boxy ?: "form-type-$type");
  if (@$box) $outerClass .= ' box1';
  if (@$attributes['disabled']) $outerClass .= ' disabled';
  
  return <<<EOF
  <div$id class="$outerClass">
$core
  </div>
EOF;
}

function rcredits_textfield($variables, $type = 'text') {
  \element_set_attributes($variables['element'], u\ray('id name value size maxlength'));
  extract(rcElement($variables, 'attributes autocomplete_path required description class'));

  if (@$required) $attributes['required'] = 'yes';
  $class[] = 'form-control input-md';
  if (is_array($ray = @$description)) {
    if (@$ray[0] and !@$ray[2]) $ray[2] = $ray[0]; // hover text defaults to same as hint
    foreach (['placeholder', 'desc', 'title'] as $i => $k) {
      if ($k != 'desc' and $v = $ray[$i]) $attributes[$k] = $v;
    }
  }    

  if ($extra = rcAuto(@$autocomplete_path, $attributes['id'])) $class[] = 'form-autocomplete';

  $tribs = u\tribs(compact('type', 'class') + $attributes);
  return "<input $tribs />" . $extra;
}

function rcredits_radios($variables) {
  $chn = $variables['element']['#children'];
  return @$variables['element']['#inline'] ? str_replace(' radio">', ' radio-inline">', $chn) : $chn;
}

function rcredits_submit($variables) {
  extract(rcElement($variables, 'tabled value title id'));
  return <<<EOF
  <button type="submit" id="$id" class="col-xs-offset-2 col-sm-offset-3 btn btn-primary btn-md ladda-button" data-style="expand-right">
    <span class="ladda-label">$value</span>
  </button>
EOF;
}

function rcredits_button($variables) {
  extract(rcElement($variables, 'tabled value href button_type class'));
  if ($button_type == 'submit') return rcredits_submit($variables);
  
  if (strpos($href, 'http') === FALSE) $onclick = $href;
  return u\tag(@$href ? 'a' : 'button', $value, compact(@$onclick ? 'onclick' : 'href', 'class'));
}

function rcredits_checkbox($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', 'value'));
  extract(rcElement($variables, 'attributes checked class default_value'));

  $type = 'checkbox';
  if (@$checked or @$default_value) $checked = 'checked'; else unset($checked);
  $tribs = u\tribs(compact('type', 'checked', 'class') + $attributes);
  return "<input $tribs />";
}

function rcredits_checkboxes($variables) {
  \element_set_attributes($variables['element'], array('id'));
  extract(rcElement($variables, 'choices defaults aliases parents'));
  $for = strtr(@$parents[0], [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']);

  // get children first
  $type = 'checkbox';
  $fields = u\ray('name value checked title class');
  $chn = '';
  foreach ($choices as $k => $v) {
    $name = "{$for}[$k]";
    $value = $k;
    $checked = in_array($v, $defaults);
    $title = $aliases[$k];
    $chn .= @\render(w\formField('checkbox', '', '', compact($fields)));
  }
  extract(rcElement($variables, 'attributes class'));
  $class[] = 'form-checkboxes';
  return u\tag('div', $chn, $attributes + compact('class'));
}

function rcredits_select($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', 'size'));
  extract(rcElement($variables, 'attributes class'));
  $class[] = 'form-control';
  return u\tag('select', \form_select_options($variables['element']), $attributes + compact('class'));
}

function rcredits_fieldset($variables) {
  \element_set_attributes($variables['element'], array('id'));
  extract(rcElement($variables, 'attributes class title description value children'));

  $class[] = 'form-wrapper';
  if (@$title) $title = u\tag('legend', u\tag('span', $title, ['class'=>'fieldset-legend']));
  if (@$description) $description = u\tag('div', $description, ['class'=>'fieldset-description']);
  $wrapper = u\tag('div', @$description . $children . @$value, ['class'=>'fieldset-wrapper']);
  return u\tag('fieldset', @$title . $wrapper, $attributes + compact('class'));
}

function rcredits_password($variables) {
  return rcredits_textfield($variables, 'password');
}

function rcredits_breadcrumb($variables) {
  $cnt = count($breadcrumb = @$variables['breadcrumb']);
  if ($cnt < 2) return '';
  $map = ['Sadmin' => t('Sys Admin'), 'Account' => t('Settings')];
  $head = '<h2 class="sr-only">' . t('You are here') . '</h2>';
  foreach ($breadcrumb as $k => $v) $crumbs[] = l(str_replace('-', ' ', strtr($k, $map)), BASE_URL . $v);
  $crumbs[$cnt - 1] = strtr($k, $map);
  return $head . '<li>' . join('</li><li>', $crumbs) . '</li>';
}

/**
 * Return assoc of element parameters, plainly named.
 */
function rcElement($variables, $fields) {
  foreach ($variables['element'] as $k => $v) $e[substr($k, 1)] = $v;
  return u\just($fields, $e);
}

/**
 * Return autocomplete HTML, if appropriate.
 * @param string $path: autocomplete URL, if any
 * @param string $id: id of the associated text input element
 * @return HTML (empty if path is empty)
 */ 
function rcAuto($path, $id) {
  if (!$path) return '';
  \drupal_add_library('system', 'drupal.autocomplete');

  $type = 'hidden';
  $id .= '-autocomplete';
  $value = url($path, array('absolute' => TRUE));
  $disabled = 'disabled';
  $class = 'autocomplete';
  $tribs = u\tribs(compact(u\ray('type id value disabled class')));
  return "<input $tribs />";
}
