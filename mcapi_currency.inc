<?php
namespace rCredits\mcapi;

function currency() {
  return array(
    'info' => array(
      'currcode' => 'currcode',
    ),
    'human_name' => 'Currencyname',
    'display' => array(
      'format' => '@minus@integer.@subdivision',
      'divisions' => '1',
      'divisions_setting' => '',
    ),
    'additional_settings' => array(
      'additional_settings__active_tab' => 'edit-currency-access',
    ),
    'currency_access' => array(
      'membership' => 'user_access_transact',
      'user_aggregated_view' => 'user_access_transact',
      'currency_aggregated_view' => 'mcapi_access_anyone',
      'undo' => 'user_access_manage_all_transactions',
    ),
    'transaction_access' => array(
      1 => array(
        'mcapi_access_authenticated' => 'mcapi_access_authenticated',
        'mcapi_xacc_participants' => 0,
        'mcapi_access_accountant' => 0,
        'mcapi_access_anyone' => 0,
      ),
      2 => array(
        'mcapi_xacc_participants' => 0,
        'mcapi_access_authenticated' => 0,
        'mcapi_access_accountant' => 0,
        'mcapi_access_anyone' => 0,
      ),
    ),
    'zero' => 1,
    'buttons' => array(
      'delete' => 'Delete',
    ),
    'uid' => '1',
  );
}