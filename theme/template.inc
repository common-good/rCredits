<?php
use CG as r;
use CG\Util as u;
use CG\Web as w;

/**
 * @file
 * rCredits template functions, called from the rCredits theme's template.php stub
 * (run Handy|Empty Cache to activate a new function here)
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
 *//*
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
*/

/**
 * Returns HTML for a form.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #action, #method, #attributes, #children
 *
 * @ingroup themeable
 */
function rcredits_form($variables) {
  global $base_url;
  $element = $variables['element'];
  if (isset($element['#action'])) {
    if ($element['#action'][0] == '/') $element['#action'] = $base_url . $element['#action']; // CGF
    $element['#attributes']['action'] = drupal_strip_dangerous_protocols($element['#action']);
  }

  $element['#id'] = str_replace('cgwebform', 'frm-', $element['#id']); // CGF
  element_set_attributes($element, array('method', 'id'));
  if (empty($element['#attributes']['accept-charset'])) {
    $element['#attributes']['accept-charset'] = "UTF-8";
  }
  // Anonymous DIV to satisfy XHTML compliance.
  return w\tags('form', w\tags('div', $element['#children']), $element['#attributes']);
//  return '<form' . u\tribs($element['#attributes']) . '><div>' . $element['#children'] . '</div></form>';
}

function rcredits_links($variables) {
  global $rUrl, $base_url;
  $links = $variables['links'];
  $attributes = $variables['attributes'];
  $attributes['class'][] = 'nav navbar-nav'; // cgf
  $menuLinks = '';
  $num_links = count($links);
  $i = 1;

  foreach ($links as $key => $item) { // for each top-level menu item
    extract($item, EXTR_PREFIX_ALL, 'm'); // get title and href

    $id = "menu-$m_href";
    $class = 'topmenu';
    if ($i == 1) $class .= ' first';
    if ($i == $num_links) $class .= ' last';
    if ($m_href == $_GET['q'] or ($m_href == '<front>' && \drupal_is_front_page())) $class .= ' active';
    
    if (in($m_href, 'history settings community sadmin')) {
      $class .= ' submenu';
      $menu = w\subMenuHtml($m_href);
      $link = <<<EOF
      <div class="popmenu">$menu</div>
      <a href="#" data-toggle="popover" data-html="true" data-trigger="manual" data-animation="false">$m_title</a>
EOF;
//      <a href="#" data-toggle="popover" data-html="true" data-trigger="manual" data-animation="false" onclick="jQuery('.submenu a').not(jQuery(this)).popover('hide'); jQuery(this).popover('toggle');">$m_title</a>
    } else $link = spinLink("/$m_href", $m_title, $id, '', ''); // the '' args are important

    $menuLinks .= "<li id=\"$id\" class=\"$class\">$link</li>\n";
    $i++;
  }

  if (function_exists('\\CG\\acct') and $mya = r\acct()) {
    $acctName = "$mya->fullName ($mya->mainQid)";
    $co = $mya->co ? ' co' : '';
  } else $acctName = $co = '';

  $breadcrumb = rcredits_breadcrumb(['breadcrumb' => \drupal_get_breadcrumb()]);
  $logo = BASE_URL . R_PATH . '/images/' . PROJECT_LOGO . '?' . R_VERSION;
  
  return <<<EOF
<div class="nav2">
  <div class="nav2-inner">
    <div id="extras" class="">
      <button type="button" class="accounts-toggle photo$co" data-toggle="collapse" data-target="#edit-acct-account" aria-expanded="false" aria-controls="edit-acct-account">
        <a title="$acctName" data-toggle="popover" data-placement="left"><img src="$base_url/account-icon"/></a>
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
      <a class="navbar-brand" href="#"><div>
        <img src="$logo" />
        <div id="orgname">Common Good<sup>&trade;</sup></div>
      </div></a>
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
  extract(rcElement($variables, 'name tabled type bare markup help parents children required title title_display id field_prefix field_suffix inline class box'));

  $parentName = strtr(@$parents[0], [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']);
  $for = @$name ?: $parentName;
  if ($for == 'title') $children = "<h3>$children</h3>";
  $children = @$field_prefix . $children . @$field_suffix;

  u\setDft($title_display, 'before');
//  $boxy = in_array($type, ['radio', 'checkbox']) ? ($type . (@$inline ? '-inline' : '')) : '';
  $boxy = in($type, 'radio checkbox') ? $type : '';

  $class[] = $boxy ? 'option' : 'control-label col-sm-offset-1 col-xs-2';
  if ($title_display == 'invisible') $class[] = 'sr-only';
  $label = w\tag('label', $boxy ? $children . @$title : @$title, compact('for', 'class'));
  if (@$bare) return $boxy ? $label : $children;
    
  if (@$tabled) {
    if (is_array(@$help)) $help = @$help[1]; // ignore hint here (see rcredits_textfield)
    $help = @$help ? "\n      <div class=\"help-block\">$help</div>" : '';
    $lineup = is_null(@$markup) ? '' : ' lineup';
    $core = <<<EOF
    $label
    <div class="control-data col-xs-10 col-sm-6$lineup">
      $children$help
    </div>
EOF;
  } else $core = $boxy ? $label : $label . $children;

  $id = @$markup ? " id=\"edit-$for\"" : '';
  $outerClass = "form-group form-item-$for";
  if ($for != $parentName) $outerClass .= " form-item-$parentName";
  $outerClass .= ' ' . ($boxy ?: "form-type-$type");
  $required = @$required ? ' required="yes"' : '';
  if (@$class[0] == 'boxes') $outerClass .= ' boxes';
  if (@$attributes['disabled']) $outerClass .= ' disabled';
  return <<<EOF
  <div$id class="$outerClass"$required>
$core
  </div>
EOF;
}

function rcredits_textfield($variables, $type = 'text') {
  \element_set_attributes($variables['element'], ray('id name value size maxlength'));
//  extract(rcElement($variables, 'attributes autocomplete_path required help class submitButton'));
  extract(rcElement($variables, 'attributes required help class submitButton'));

  if (@$submitButton) return rcredits_submit($variables);
  if (@$required) $attributes['required'] = 'yes'; // don't add this in unless it is yes
  $class[] = 'form-control input-md';
  if (is_array($ray = @$help)) {
    if (@$ray[0] and !@$ray[2]) $ray[2] = $ray[0]; // hover text defaults to same as hint
    foreach (['placeholder', 'desc', 'title'] as $i => $k) {
      if ($k != 'desc' and $v = $ray[$i]) $attributes[$k] = $v;
    }
  }    

//  if ($extra = rcAuto(@$autocomplete_path, @$attributes['id'])) $class[] = 'form-autocomplete';

  $tribs = u\tribs(compact('type', 'class') + $attributes);
//  return "<input $tribs />" . $extra;
  return "<input $tribs />";
}

function rcredits_textarea($variables) {
  \element_set_attributes($variables['element'], array('id', 'name'));
  extract(rcElement($variables, 'attributes required name help class default_value value'));

  u\setDft($value, @$default_value); // allow either val or dft(val) in w\areaFld()
  $value = plain(@$value);
  if (@$required) $attributes['required'] = 'yes';
  $class[] = 'form-control input-md';
  if (is_array($ray = @$help)) {
    if (@$ray[0] and !@$ray[2]) $ray[2] = $ray[0]; // hover text defaults to same as hint
    foreach (['placeholder', 'desc', 'title'] as $i => $k) {
      if ($k != 'desc' and $v = $ray[$i]) $attributes[$k] = $v;
    }
  }    
  $tribs = u\tribs(compact('class') + (@$attributes ?: []));
  return "<textarea $tribs />$value</textarea>";
}

function rcredits_radios($variables) {
  $chn = $variables['element']['#children'];
  extract(rcElement($variables, 'required inline'));
  if (@$inline) $chn = str_replace(' radio">', ' radio-inline">', $chn);
  if (@$required) $chn = str_replace('type="radio"', 'type="radio" required="yes"', $chn);
  return $chn;
}

function rcredits_radio($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', '#return_value' => 'value'));
  extract(rcElement($variables, 'title attributes class return_value value parents'));
  
  if (isset($return_value) and $value == $return_value) $checked = 'checked';
  $type = 'radio';
  $class[] = 'form-radio';
  $tribs = u\tribs(compact(ray('type class checked')) + $attributes);
  if (@$checked) f('t.setRadio', $parents[0], $title);

  return "<input $tribs />";
}

function rcredits_submit($variables) {
  extract(rcElement($variables, 'tabled bare value title id size style parents class'));
//  if (!@$id) $id = 'edit-' . strtolower(strtr($value, [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']));
  $id2 = 'edit-' . @$parents[0]; // whether or not id is set (assures mixed case when appropriate
  if (!@$id or $id == strtolower($id2)) $id = $id2;
///    if ($id == 'edit-opencompany') die(print_r(compact('parents','id'), 1));
//  u\setDft($id, strtolower("edit-$value"));
//  u\setDft($size, 'md');
//  u\setDft($style, 'primary');
//  u\setDft($name, @$parents[0]);
  $variables['element']['#children'] = $res = spinLink('submit', $value, $id, @$style, @$size, compact('class'));
/*
  <<<EOF
  <button type="submit" id="$id" name="$name" value="$value" class="btn btn-$style btn-$size ladda-button" data-style="expand-right">
    <span class="ladda-label">$value</span>
  </button>
EOF;
*/
  return @$bare ? $res : rcredits_form_element($variables);
}

function rcredits_button($variables) {
  extract(rcElement($variables, 'tabled value href button_type class'));
  if ($button_type == 'submit') return rcredits_submit($variables);
  
/**/ if (strpos($href, 'http') === FALSE) die('no http in button'); // $onclick = $href;
  return w\tag(@$href ? 'a' : 'button', $value, compact(@$onclick ? 'onclick' : 'href', 'class'));
}

function rcredits_checkbox($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', 'value'));
  extract(rcElement($variables, 'title attributes checked class default_value'));

  $type = 'checkbox';
  if (@$checked or @$default_value) $checked = 'checked'; else unset($checked);
  if (@$checked) f('t.setChecked', $title);
  $tribs = u\tribs(compact('type', 'checked', 'class') + $attributes);
  return "<input $tribs />";
}

function rcredits_checkboxes($variables) {
  \element_set_attributes($variables['element'], array('id'));
  extract(rcElement($variables, 'choices defaults aliases parents'));
  $for = strtr(@$parents[0], [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']);

  // get children first
  $type = 'checkbox';
  $fields = ray('name value checked title class');
  $chn = '';
  foreach ($choices as $k => $v) {
    $name = "{$for}[$k]";
    $value = $k;
    $checked = in($k, $defaults);
///    debug(compact(ray('k v defaults checked')));
    $title = $aliases[$k];
    $chn .= @\render(w\fld('checkbox', '', '', compact($fields)));
  }
  extract(rcElement($variables, 'attributes class'));
  $class[] = 'form-checkboxes';
  return w\tag('div', $chn, $attributes + compact('class'));
}

function rcredits_select($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', 'size'));
  extract(rcElement($variables, 'attributes class'));
  $class[] = 'form-control';
  return w\tag('select', \form_select_options($variables['element']), ($attributes ?: []) + compact('class'));
}

function rcredits_fieldset($variables) {
  \element_set_attributes($variables['element'], array('id'));
  extract(rcElement($variables, 'attributes class legend bare help value children'));

  $class[] = 'form-wrapper';
  u\preray(compact('class'), $attributes);
  if (@$legend) $legend = w\tag('legend', w\tag('span', $legend, ['class'=>'fieldset-legend']));
//  if (@$help) $help = w\tag('div', $help, ['class'=>'fieldset-help']);
  $wrapper = w\tag('div', $children . @$value, ['class'=>'fieldset-wrapper']);
//  return w\tag('fieldset', @$title . $wrapper, $attributes + compact('class'));
  $variables['element']['#children'] = w\tag('fieldset', @$legend . $wrapper, $attributes);
  return rcredits_form_element($variables);
}

function rcredits_password($variables) {
  return rcredits_textfield($variables, 'password');
}

function rcredits_breadcrumb($variables) {
  $cnt = count($breadcrumb = @$variables['breadcrumb']);
  if ($cnt < 2) return '';
  $map = ['Sadmin' => t('Sys Admin'), 'Account' => t('Settings')];
  $head = '<h2 class="sr-only">' . t('You are here') . '</h2>';
  foreach ($breadcrumb as $k => $v) {
    $k = ucwords(str_replace('-', ' ', strtr($k, $map)));
    if ($i = strpos($k, '?')) $k = substr($k, 0, $i);
    $crumbs[] = l($k, BASE_URL . $v);
  }
  $crumbs[$cnt - 1] = $k;
  return $head . '<li>' . join('</li><li>', $crumbs) . '</li>';
}

function rcredits_hidden($variables) {
  \element_set_attributes($variables['element'], array('name', 'value'));
  $type = 'hidden';
  extract(rcElement($variables, 'attributes id'));
  $tribs = u\tribs(compact(ray('type id value')) + $attributes);
  return "<input $tribs />\n";
}

/**
 * Return assoc of element parameters, plainly named.
 */
function rcElement($variables, $fields) {
  foreach ($variables['element'] as $k => $v) $e[substr($k, 1)] = $v;
  return just($fields, $e);
}

/**
 * Return autocomplete HTML, if appropriate.
 * @param string $path: autocomplete URL, if any
 * @param string $id: id of the associated text input element
 * @return HTML (empty if path is empty)
 */ /*
function rcAuto($path, $id) {
  if (!$path) return '';
//  \drupal_add_library('system', 'drupal.autocomplete');

  $type = 'hidden';
  $id .= '-autocomplete';
  $value = url($path, array('absolute' => TRUE));
  $disabled = 'disabled';
  $class = 'autocomplete';
  $tribs = u\tribs(compact(ray('type id value disabled class')));
  return "<input $tribs />";
}*/

/**
 * Return HTML for a button-like link with a spinner.
 * @param string $href: where to go.
 */
function spinLink($href, $text, $id = '', $style = 'primary', $size = 'md', $other = []) {
  $dataStyles = ray('zoom-in zoom-out slide-left slide-right'); // slide-up slide-down don't work well on menus
  $dataStyle = $dataStyles[rand(0, count($dataStyles) - 1)];
  list ($tag, $class, $attrs) = ['a', "btn btn-$style btn-$size", 'href id']; // default tag, class, and attribs
//  if ($id == 'edit-nextStep') $class .= ' center-block';
  if (!$id) unset($id);
  
  if ($href == 'submit') { // submit button
    list ($type, $value, $name, $tag) = ['submit', $text, 'op', 'button']; // name must be "op" for Drupal
    $attrs = 'type name id value';
  } else { // menu (or list) -type link (not a button)
    if ($href[0] == '/') $href = BASE_URL . $href;
    if (strpos($href, BASE_URL) !== 0 and substr($href, 0, 4) == 'http')  $other += w\away();
    if (!$size) $class = $style;
  }

  if ($class2 = @$other['class']) {$class .= ' ' . (is_array($class2) ? join(' ', $class2) : $class2); unset($other['class']);}
  if (!@$other['target']) $class .= ' ladda-button'; // don't spin if opening a window

//  foreach ($other + compact(ray($attrs)) as $k => $v) if ($v = plain($v)) $attrs2 = @$attrs2 . " $k=\"$v\"";
  $attrs2 = u\tribs($other + compact('class', ray($attrs)));
  //$attrs2 = \drupal_attributes($other + compact(ray($attrs)));

  return <<<EOF
  <$tag $attrs2 data-style="$dataStyle">
    <span class="ladda-label">$text</span>
  </$tag>
EOF;
}