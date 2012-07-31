<?php
namespace rCredits\Web;
use \rCredits as r;
/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

function pay_form($form, &$form_state) {
  $form = array(
    'description' => form_field('item', t('Payment Form')),
    'amount' => form_field('textfield', '$', t('Amount to pay'), 'required:1'),
    'other' => form_field('textfield', 'to:', t('Pay whom?'), 'required:1'),
    'what' => form_field('textfield', 'for:', t('Description of goods and services'), 'required:1'),
  );
  return $form;
}
 
function form_field($type, $title, $description = '', $other = '', $ajax = array()) {
  $field = r\prefix_keys(array_merge(compact(r\ray('type title description')), r\ray($other)), '#');
  return $field;
}

// one-line functions that need no explanation
function block_def($info, $cache = DRUPAL_CACHE_PER_ROLE) {return compact(r\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(r\ray('subject content'));}
