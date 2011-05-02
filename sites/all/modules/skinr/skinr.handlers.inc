<?php
/**
 * @file
 * Defines the various default handler functions to support Skinr.
 */

/*
 * Skinr access handler.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - access skinr: Access to edit Skinr's settings.
 *   - access skinr classes: Access to edit Skinr's additional classes.
 * @param &$form
 *   Passes in the $form parameter from hook_form_alter().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_alter().
 *
 * @return
 *   TRUE if we get access, FALSE otherwise.
 */
function skinr_access_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'edit skin settings':
      return user_access('edit skin settings') || user_access('administer skinr');
    case 'edit advanced skin settings':
      return user_access('edit advanced skin settings') || user_access('administer skinr');
  }
}

/**
 * Skinr form index handler.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - form: The form elements for Skinr are being inserted in a form.
 *   - submit: The form has been submitted.
 * @param &$form
 *   - For 'form', passes in the $form parameter from hook_form_alter().
 *   - For 'submit', passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   - For 'form', passes in the $form_state parameter from hook_form_alter().
 *   - For 'submit', passes in the $form_state parameter from
 *     hook_form_submit().
 *
 * @return
 *   The index where we can find our values in Skinr's data structure.
 */
function skinr_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      if (empty($form['skinr']['sid']['#value'])) {
        trigger_error(sprintf("The form with form_id '%' is not a valid skinr form.", $form['form_id']['#value']), E_USER_ERROR);
        return FALSE;
      }
      return $form['skinr']['sid']['#value'];

    case 'submit':
      if (empty($form_state['values']['sid'])) {
        trigger_error(sprintf("The form with form_id '%' is not a valid skinr form.", $form['form_id']['#value']), E_USER_ERROR);
        return FALSE;
      }
      return $form_state['values']['sid'];
  }
}

/**
 * Skinr data handler.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_submit().
 * @param $theme
 *   The theme that is currently being processed.
 * @param $module
 *   The module that is currently being processed.
 * @param $form_settings
 *   The settings from hook_skinr_config() for the form that's currently being
 *   processed.
 *
 * @return
 *   A skinr settings object.
 */
function skinr_data_handler(&$form, $form_state, $theme, $module, $form_settings) {
  // Ensure we have the required index_handler
  if (empty($form_settings['index_handler'])) {
    trigger_error(sprintf("No index_handler was found for form_id '%s' in module '%s'.", $form_id, $module), E_USER_ERROR);
  }
  $index = skinr_handler('form_index_handler', 'form', $form_settings['index_handler'], $form, $form_state);

  return skinr_skin_load($theme, $module, $index);
}

/**
 * Skinr submit handler.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_submit().
 * @param $module
 *   The module that is currently being processed.
 * @param $form_settings
 *   The settings from hook_skinr_config() for the form that's currently being
 *   processed.
 */
function skinr_submit_handler(&$form, $form_state, $module, $form_settings) {
  if (isset($form_state['values']['skinr_settings'][$module . '_group'])) {
    foreach ($form_state['values']['skinr_settings'][$module . '_group'] as $theme_name => $theme) {
      if ((!empty($theme['widgets']) && is_array($theme['widgets'])) || isset($theme['_additional'])) {
        $skin = new stdClass();
        $skin->theme = $theme_name;
        $skin->module = $module;
        $skin->sid = skinr_handler('form_index_handler', 'submit', $form_settings['index_handler'], $form, $form_state);
        $skin->skins = array();

        if (!empty($theme['widgets']) && is_array($theme['widgets'])) {
          foreach ($theme['widgets'] as $skin_name => $skin_value) {
            $skin->skins['_options'][$skin_name] = $skin_value;
          }
        }

        if (isset($theme['_additional'])) {
          $theme['_additional'] = trim($theme['_additional']);
          if (!empty($theme['_additional'])) {
            $skin->skins['_additional'] = $theme['_additional'];
          }
        }

        if (empty($skin->sid)) {
          // We didn't receive a valid sid, so raise an error
          drupal_set_message(t("Skinr settings weren't saved due to an error."), 'error');
        }

        skinr_skin_save($skin);
      }
    }
  }
}

/**
 * Implements hook_skinr_api_VERSION().
 */
function skinr_skinr_api_2() {
}

/**
 * Helper function for built-in integration code.
 */
function skinr_skinr_api_modules() {
  return array(
    'path' => drupal_get_path('module', 'skinr') . '/modules',
  );
}

function block_skinr_api_2() {
  return skinr_skinr_api_modules();
}

function comment_skinr_api_2() {
  return skinr_skinr_api_modules();
}

function node_skinr_api_2() {
  return skinr_skinr_api_modules();
}

function panels_skinr_api_2() {
  return skinr_skinr_api_modules();
}

function views_skinr_api_2() {
  return skinr_skinr_api_modules();
}
