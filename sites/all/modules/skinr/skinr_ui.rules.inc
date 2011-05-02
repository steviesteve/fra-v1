<?php

/**
 * @file
 * Admin page callbacks for Skinr module's rules.
 */

/**
 * Menu callback; displays the skinr rules listing.
 */
function skinr_rules() {
  $output = '';

  $headers = array(
    array('data' => t('Title'), 'field' => 'title'),
    array('data' => t('Type'), 'field' => 'type'),
    array('data' => t('Operations'), 'colspan' => 2)
  );

  $rules = skinr_rule_load_multiple();
  $rows = array();
  foreach ($rules as $rule) {
    $row = array(
      check_plain($rule->title),
      check_plain($rule->rule_type),
      l(t('edit'), 'admin/appearance/skinr/rules/edit/'. $rule->rid),
      l(t('delete'), 'admin/appearance/skinr/rules/delete/'. $rule->rid),
    );
    $rows[] = $row;
  }

  $link = l(t('Create a new rule'), 'admin/appearance/skinr/rules/add');
  $row = array();
  if (empty($rows)) {
    $row[] = array(
      'data' => t('No rules have been set up yet. !url.', array('!url' => $link)),
      'colspan' => 4,
    );
  }
  else {
    $row[] = array(
      'data' => t('!url.', array('!url' => $link)),
      'colspan' => 4,
    );
  }
  $rows[] = $row;

  $output .= theme('table', array('header' => $headers, 'rows' => $rows));
  $output .= drupal_render($form);

  return $output;
}

/**
 * Menu callback; displays the edit form for a skinr rule.
 *
 * @ingroup forms
 */
function skinr_rule_add($form, &$form_state) {
  $form = array();
  $form['#tree'] = TRUE;

  $form['rule']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => !empty($form_state['values']['rule']['title']) ? $form_state['values']['rule']['title'] : '',
    '#description' => t('Descriptive title for this rule; used by administrators.'),
    '#required' => TRUE,
  );
  $options = array('page' => t('Page'));
   foreach (list_themes() as $theme_name => $theme) {
     if (empty($theme->status)) {
       continue;
     }
     // Create a list options containing visible regions of this theme.
     $regions = array();
     foreach (system_region_list($theme_name, REGIONS_VISIBLE) as $region_name => $region) {
       $regions['region__' . $region_name] = $region;
     }
     // Group the list of options by theme.
     $key = t('@name Regions', array('@name' => $theme->info['name']));
     $options[$key] = $regions;
  }
  $form['rule']['rule_type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $options,
    '#default_value' => !empty($form_state['values']['rule']['rule_type']) ? $form_state['values']['rule']['rule_type'] : '',
    '#description' => t('Type of element the rule is applied to.'),
    '#required' => TRUE,
  );

  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 * Process skinr_rule_add() submissions.
 */
function skinr_rule_add_submit($form, &$form_state) {
  $rule = new stdClass();
  $rule->rid = NULL;
  $rule->rule_type = $form_state['values']['rule']['rule_type'];
  $rule->title = $form_state['values']['rule']['title'];
  $rule->roles = array();
  $rule->visibility = 0;
  $rule->pages = '';

  skinr_rule_save($rule);
  // Set rule id, if we inserted a new rule to allow others to know what rule they're working with.
  $form_state['values']['rule']['rid'] = $rule->rid;

  $form_state['redirect'] = 'admin/appearance/skinr/rules/edit/' . $rule->rid;
}

/**
 * Menu callback; displays the edit form for a skinr rule.
 *
 * @param $rid
 *   The rule ID.
 *
 * @ingroup forms
 */
function skinr_rule_edit($form, &$form_state, $rid = NULL) {
  $form['rule'] = array(
    '#type' => 'fieldset',
    '#title' => t('Skinr rule visibility'),
    '#collapsible' => TRUE,
    '#weight' => -1,
  );

  if (isset($form_state['values'])) {
    $rule = array(
      'title' => $form_state['values']['title'],
      'rule_type' => $form_state['values']['rule_type'],
      'roles' => $form_state['values']['roles'],
      'visibility' => $form_state['values']['visibility'],
      'pages' => $form_state['values']['pages'],
    );
  }
  elseif (isset($rid) && $rule = skinr_rule_load($rid)) {
    $rule = (array) $rule;
    $form['rule']['rid'] = array(
      '#type' => 'value',
      '#value' => $rid,
    );
  }
  else {
    $rule = array(
      'title' => '',
      'rule_type' => '',
      'roles' => array(),
      'visibility' => 0,
      'pages' => '',
    );
  }

  $form['rule']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $rule['title'],
    '#description' => t('Descriptive title for this rule; used by administrators.'),
    '#required' => TRUE,
  );

  $form['rule']['rule_type'] = array(
    '#type' => 'hidden',
    '#value' => $rule['rule_type'],
  );
  $form['rule']['rule_type_displayed'] = array(
    '#type' => 'item',
    '#title' => t('Type'),
    '#markup' => $rule['rule_type'],
    '#description' => t('Type of element the rule is applied to.'),
  );

  // Inject the rule's form elements here.
  $form['rule']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Show for specific roles'),
    '#default_value' => $rule['roles'],
    '#options' => user_roles(),
    '#description' => t('Show only for the selected role(s). If you select no roles, it will be visible to all users.'),
  );

  // Set up options and description.
  $options = array(0 => t('Show on every page except the listed pages.'), 1 => t('Show on only the listed pages.'));
  $description = t("Enter one page per line as Drupal paths. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'));

  // Add the PHP specific stuff, if user has access.
  if (module_exists('php') && user_access('use PHP for visibility')) {
    $options[2] = t('Show if the following PHP code returns <code>TRUE</code> (PHP-mode, experts only).');
    $description .= ' '. t('If the PHP-mode is chosen, enter PHP code between %php. Note that executing incorrect PHP-code can break your Drupal site.', array('%php' => '<?php ?>'));
  }
  $form['rule']['visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Show on specific pages'),
    '#options' => $options,
    '#default_value' => $rule['visibility'],
    '#required' => TRUE,
  );
  $form['rule']['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#default_value' => $rule['pages'],
    '#description' => $description,
  );

  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if (isset($rule['rid'])) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('skinr_rule_delete_submit'),
    );
  }

  return $form;
}

/**
 * Process skinr_rule_edit() submissions.
 */
function skinr_rule_edit_submit($form, &$form_state) {
  $rule = new stdClass();
  $rule->rid = !empty($form_state['values']['rid']) ? $form_state['values']['rid'] : NULL;
  $rule->rule_type = $form_state['values']['rule_type'];
  $rule->title = $form_state['values']['title'];
  $rule->roles = $form_state['values']['roles'];
  $rule->visibility = $form_state['values']['visibility'];
  $rule->pages = $form_state['values']['pages'];

  skinr_rule_save($rule);
  // Set rule id, if we inserted a new rule to allow others to know what rule they're working with.
  $form_state['values']['rid'] = $rule->rid;

  $form_state['redirect'] = 'admin/appearance/skinr/rules';
}

/**
 * Called from within the rule edit form; redirects to skinr_rule_delete_confirm().
 *
 * @ingroup forms
 */
function skinr_rule_delete_submit($form, &$form_state) {
  $destination = '';
  if (isset($_REQUEST['destination'])) {
    $destination = drupal_get_destination();
    unset($_REQUEST['destination']);
  }
  $form_state['redirect'] = array('admin/appearance/skinr/rules/delete/'. $form_state['values']['rid'], $destination);
}

/**
 * Menu callback; displays the delete confirmation for a skinr rule.
 *
 * @param $rid
 *   The rule ID.
 *
 * @ingroup forms
 */
function skinr_rule_delete_confirm($form, &$form_state, $rid) {
  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $rid,
  );

  $rule = skinr_rule_load($rid);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $rule->title)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/appearance/skinr/rules',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Process skinr_rule_delete_confirm() submissions.
 */
function skinr_rule_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    skinr_rule_delete($form_state['values']['rid']);
  }

  $form_state['redirect'] = 'admin/appearance/skinr/rules';
}
