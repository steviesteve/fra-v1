<?php
/**
 * @file
 * Provide skinr handling for block.module
 */

/**
 * @defgroup skinr_block_module block.module handlers
 *
 * @{
 */

/**
 * Implements hook_skinr_config_info().
 */
function block_skinr_config_info() {
  $data['block']['form']['block_admin_configure'] = array(
    'index_handler' => 'block_skinr_form_index_handler',
    'preprocess_hook_callback' => 'block_skinr_preprocess_hook_callback',
    'title' => t('block settings'),
    'skinr_weight' => 0,
    'collapsed' => FALSE,
  );
  $data['block']['form']['block_add_block_form'] = array(
    'index_handler' => 'block_skinr_form_index_handler',
    'preprocess_hook_callback' => 'block_skinr_preprocess_hook_callback',
    'title' => t('block settings'),
    'skinr_weight' => 0,
    'collapsed' => FALSE,
  );
  $data['block']['form']['skinr_ui_form'] = array(
    'preprocess_hook_callback' => 'block_skinr_preprocess_hook_callback',
    'title' => t('block settings'),
    'collapsed' => FALSE,
  );
  $data['block']['preprocess']['block'] = array(
    'index_handler' => 'block_skinr_preprocess_index_handler',
  );
  $data['block']['contextual_links']['block'] = array(
    'contextual_links_handler' => 'block_skinr_contextual_links',
  );

  return $data;
}

/**
 * Skinr form index handler.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - 'form': the form elements for Skinr are being inserted in a form.
 *   - 'submit': the form has been submitted.
 * @param &$form
 *   - For 'form', passes in the $form parameter from hook_form_alter().
 *   - For 'submit', passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   - For 'form', passes in the $form_state parameter from hook_form_alter().
 *   - For 'submit', passes in the $form_state parameter from hook_form_submit().
 *
 * @return
 *   The index where we can find our values in Skinr's data structure.
 */
function block_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      return $form['module']['#value'] . '-' . $form['delta']['#value'];

    case 'submit':
      return $form_state['values']['module'] . '-' . $form_state['values']['delta'];
  }
}

/**
 * Skinr preprocess hook callback.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_alter().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_alter().
 *
 * @return
 *   An array of preprocess hooks we wish to use.
 */
function block_skinr_preprocess_hook_callback(&$form, $form_state) {
  $preprocess_hooks = array();

  if (empty($form['module']['#value']) && !empty($form['skinr']['sid']['#value'])) {
    $result = db_select('block', 'b')
      ->fields('b')
      ->distinct()
      ->where('CONCAT(b.module, \'-\', b.delta) = :identifier', array(':identifier' => $form['skinr']['sid']['#value']))
      ->range(0, 1)
      ->execute();
    foreach ($result as $block) {
      $preprocess_hooks[] = 'block_'. $block->module;
    }
  }
  else {
    $preprocess_hooks[] = 'block_'. $form['module']['#value'];
  }
  $preprocess_hooks[] = 'block';

  return $preprocess_hooks;
}

/**
 * Skinr preprocess index handler.
 *
 * @param &$variables
 *   Passes in the $variables parameter from module_preprocess().
 *
 * @return
 *   The index where we can find our values in Skinr's data structure. If an
 *   array is returned, it will loop through each index in Skinr's data
 *   structure and merge the returned classes.
 */
function block_skinr_preprocess_index_handler(&$variables) {
  return $variables['block']->module . '-' . $variables['block']->delta;
}

/**
 * Skinr contextual links handler.
 *
 * @param &$variables
 *   Passes in the $variables parameter from skinr_preprocess().
 *
 * @return
 *   An array. Each value is an array that forms the function arguments for
 *   menu_contextual_links(). For example:
 *   @code
 *     array(
 *       'admin/appearance/skinr/edit', array('system', 'navigation')),
 *     )
 *   @endcode
 */
function block_skinr_contextual_links(&$variables) {
  $links = array();
  $links['skinr-block'] = array(
    'admin/appearance/skinr/edit/nojs', array('block', $variables['block']->module . '-' . $variables['block']->delta),
  );
  return $links;
}

/**
 * @}
 */
