<?php

/**
 * @file
 * Admin page callbacks for the Skinr module.
 */

/**
 * Implements hook_skinr_operations().
 */
function skinr_ui_skinr_operations() {
  $operations = array(
    'delete' => array(
      'label' => t('Delete selected Skinr settings'),
      'callback' => NULL,
    ),
  );
  return $operations;
}

/**
 * List skinr administration filters that can be applied.
 *
 * @return
 *   An array of filters.
 */
function skinr_ui_filters() {
  // Theme filter.
  $themes = list_themes();
  ksort($themes);

  $options = array('[any]' => t('any'));
  foreach ($themes as $theme) {
    if (!$theme->status) {
      continue;
    }
    $options[$theme->name] = $theme->info['name'];
  }

  $filters['theme'] = array(
    'title' => t('theme'),
    'options' => $options,
  );

  // Type filter.
  $config = skinr_get_config_info();

  $options = array('[any]' => t('any'));
  foreach ($config as $type => $data) {
    $options[$type] = $type;
  }

  $filters['type'] = array(
    'title' => t('type'),
    'options' => $options,
  );

  return $filters;
}

/**
 * Form builder for the Skinr administration filters form.
 *
 * @ingroup forms
 *
 * @todo This function is missing the $form and $form_state parameters.
 */
function skinr_ui_filter_form() {
  $session = &$_SESSION['skinr_ui_overview_filter'];
  $session = is_array($session) ? $session : array();
  $filters = skinr_ui_filters();

  $i = 0;
  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Show only items where'),
    '#theme' => 'exposed_filters__skinr',
  );
  foreach ($session as $filter) {
    list($type, $value) = $filter;
    $value = $filters[$type]['options'][$value];
    $t_args = array('%property' => $filters[$type]['title'], '%value' => $value);

    if ($i++) {
      $form['filters']['current'][] = array('#markup' => t('and where %property is %value', $t_args));
    }
    else {
      $form['filters']['current'][] = array('#markup' => t('where %property is %value', $t_args));
    }
    if (in_array($type, array('theme', 'type'))) {
      // Remove the option if it is already being filtered on.
      unset($filters[$type]);
    }
  }

  $form['filters']['status'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
    '#prefix' => ($i ? '<div class="additional-filters">' . t('and where') . '</div>' : ''),
  );
  $form['filters']['status']['filters'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('filters')),
   );

  foreach ($filters as $key => $filter) {
    $names[$key] = $filter['title'];
    $form['filters']['status']['filters'][$key] = array(
      '#type' => 'select',
      '#title' => $filter['title'],
      '#options' => $filter['options'],
      '#default_value' => '[any]',
    );
  }

  $form['filters']['status']['actions'] = array(
    '#type' => 'actions',
    '#id' => 'skinr-exposed-filters',
    '#attributes' => array('class' => array('container-inline')),
  );
  if (count($filters)) {
    $form['filters']['status']['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => count($session) ? t('Refine') : t('Filter'),
    );
  }
  if (count($session)) {
    $form['filters']['status']['actions']['undo'] = array('#type' => 'submit', '#value' => t('Undo'));
    $form['filters']['status']['actions']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  drupal_add_js('misc/form.js');

  return $form;
}

/**
 * Form submission handler for skinr_ui_filter_form().
 */
function skinr_ui_filter_form_submit($form, &$form_state) {
  $filters = skinr_ui_filters();
  switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      // Apply every filter that has a choice selected other than 'any'.
      foreach ($filters as $filter => $options) {
        if (isset($form_state['values'][$filter]) && $form_state['values'][$filter] != '[any]') {
          // Flatten the options array to accommodate hierarchical/nested options.
          $flat_options = form_options_flatten($filters[$filter]['options']);
          // Only accept valid selections offered on the dropdown, block bad input.
          if (isset($flat_options[$form_state['values'][$filter]])) {
            $_SESSION['skinr_ui_overview_filter'][] = array($filter, $form_state['values'][$filter]);
          }
        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['skinr_ui_overview_filter']);
      break;
    case t('Reset'):
      $_SESSION['skinr_ui_overview_filter'] = array();
      break;
  }
}

/**
 * Form builder for the Skinr settings overview form.
 *
 * @ingroup forms
 */
function skinr_ui_list($form, &$form_state) {
  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    return skinr_ui_multiple_delete_confirm($form, $form_state, array_filter($form_state['values']['skins']));
  }
  $form['filter'] = skinr_ui_filter_form();
  $form['#submit'][] = 'skinr_ui_filter_form_submit';
  $form['admin'] = skinr_ui_admin_skins();

  return $form;
}

/**
 * Subform builder for the Skinr settings overview form.
 *
 * @ingroup forms
 */
function skinr_ui_admin_skins() {
  $session = !empty($_SESSION['skinr_ui_overview_filter']) && is_array($_SESSION['skinr_ui_overview_filter']) ? $_SESSION['skinr_ui_overview_filter'] : array();
  $filters = array();
  foreach ($session as $filter) {
    list($key, $value) = $filter;
    $filters[$key] = $value;
  }

  // Build an 'Update options' form.
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#attributes' => array('class' => array('container-inline')),
  );

  $options = array();
  $options['delete'] = t('Delete the selected Skinr settings');

  $form['options']['operation'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => 'publish',
  );
  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array('skinr_ui_admin_skins_validate'),
    '#submit' => array('skinr_ui_admin_skins_submit'),
  );

  $sorts = array();

  $headers = array(
    'theme' => array('data' => t('Theme'), 'field' => 'theme'),
    'type' => array('data' => t('Type'), 'field' => 'type'),
    'sid' => array('data' => t('Skinr ID'), 'field' => 'sid'),
    'operations' => array('data' => t('Operations')),
  );

  $ts = tablesort_init($headers);
  $sort = SORT_DESC;
  if ($ts['sort'] == 'asc') {
    $sort = SORT_ASC;
  }

  switch ($ts['sql']) {
    case 'type':
    case 'sid':
      $sortby = $ts['sql'];
      break;

    case 'theme':
    default:
      $sortby = 'theme';
      break;
  }

  $themes = list_themes();
  ksort($themes);

  $options = array();
  $destination = drupal_get_destination();
  $type = !empty($filters['type']) ? $filters['type'] : NULL;

  foreach ($themes as $theme) {
    if (!$theme->status || !empty($theme->info['hidden']) || (!empty($filters['theme']) && $filters['theme'] != $theme->name)) {
      continue;
    }
    if (empty($type)) {
      $skins = skinr_skin_load($theme->name);
    }
    else {
      $skins = array($type => skinr_skin_load($theme->name, $type));
    }

    // skinr_skin_load() could return NULL for this theme.
    if (!empty($skins)) {
      foreach ($skins as $module => $elements) {
        foreach ($elements as $sid => $skin) {
          $operations = array(
            'edit' => array(
              'title' => t('edit'),
              'href' => 'admin/appearance/skinr/edit/nojs/' . $module . '/' . $sid,
              'query' => $destination,
            ),
            'delete' => array(
              'title' => t('delete'),
              'href' => 'admin/appearance/skinr/delete/'. $theme->name .'/'. $module .'/'. $sid,
              'query' => $destination,
            ),
          );
          $options[$theme->name . '__' . $module . '__' . $sid] = array(
            'theme' => $theme->info['name'],
            'type' => $module,
            'sid' => $sid,
            'operations' => array(
              'data' => array(
                '#theme' => 'links__skinr_operations',
                '#links' => $operations,
                '#attributes' => array('class' => array('links', 'inline')),
              ),
            ),
          );
          $sorts[] = strip_tags($options[$theme->name . '__' . $module . '__' . $sid][$sortby]);
        }
      }
    }
  }

  // Sort table.
  array_multisort($sorts, $sort, $options);

  // Output table.
  $form['skins'] = array(
    '#type' => 'tableselect',
    '#header' => $headers,
    '#options' => $options,
    '#empty' => t('No content available.'),
  );
  $form['pager'] = array('#markup' => theme('pager', array('tags' => NULL)));

  return $form;
}

/**
 * Form validation handler for skinr_ui_list().
 *
 * Check if any skinr settings have been selected to perform the chosen
 * 'Update option' on.
 */
function skinr_ui_admin_skins_validate($form, &$form_state) {
  // Error if there are no items to select.
  if (!is_array($form_state['values']['skins']) || !count(array_filter($form_state['values']['skins']))) {
    form_set_error('', t('No items selected.'));
  }
}

/**
 * Form submission handler for skinr_ui_list().
 *
 * Execute the chosen 'Update option' on the selected skinr settings.
 */
function skinr_ui_admin_skins_submit($form, &$form_state) {
  $operations = module_invoke_all('skinr_operations');
  $operation = $operations[$form_state['values']['operation']];
  // Filter out unchecked nodes
  $nodes = array_filter($form_state['values']['skins']);
  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array_merge(array($nodes), $operation['callback arguments']);
    }
    else {
      $args = array($nodes);
    }
    call_user_func_array($function, $args);

    cache_clear_all();
  }
  else {
    // We need to rebuild the form to go to a second step. For example, to
    // show the confirmation form for the deletion of nodes.
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Form builder for the confirmation form when deleting multiple Skinr settings.
 *
 * @param $skins
 *   An array of skins to delete.
 *
 * @ingroup forms
 */
function skinr_ui_multiple_delete_confirm($form, &$form_state, $skins) {
  $themes = list_themes();

  $form['skins'] = array('#prefix' => '<ul>', '#suffix' => '</ul>', '#tree' => TRUE);
  // array_filter returns only elements with TRUE values
  foreach ($skins as $skin => $value) {
    $parts = explode('__', $skin, 3);
    $form['skins'][$skin] = array(
      '#type' => 'hidden',
      '#value' => $skin,
      '#prefix' => '<li>',
      '#suffix' => t('Skinr ID %sid of type %type for theme %theme', array('%sid' => $parts[2], '%type' => $parts[1], '%theme' => $themes[$parts[0]]->info['name'])) ."</li>\n",
    );
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');
  $form['#submit'][] = 'skinr_ui_multiple_delete_confirm_submit';
  $confirm_question = format_plural(count($skins),
                                  'Are you sure you want to delete this item?',
                                  'Are you sure you want to delete these items?');
  return confirm_form($form,
                    $confirm_question,
                    'admin/appearance/skinr', t('This action cannot be undone.'),
                    t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for skinr_ui_multiple_delete_confirm().
 */
function skinr_ui_multiple_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    foreach ($form_state['values']['skins'] as $skin => $value) {
      $parts = explode('__', $skin, 3);

      $skin = new StdClass();
      $skin->theme = $parts[0];
      $skin->module = $parts[1];
      $skin->sid = $parts[2];
      $skin->skins = array();

      skinr_skin_save($skin);
    }

    $count = count($form_state['values']['skins']);
    if ($count == 1) {
      watchdog('content', 'Deleted 1 skinr setting.');
    }
    else {
      watchdog('content', 'Deleted @count skinr settings.', array('@count' => $count));
    }
    drupal_set_message(format_plural($count, 'Deleted 1 skinr setting.', 'Deleted @count skinr settings.'));
  }
  $form_state['redirect'] = 'admin/appearance/skinr';
}

/**
 * Form builder for the skin info listing form.
 *
 * @ingroup forms
 * @see skinr_ui_admin_skin_infos_submit()
 */
function skinr_ui_admin_skin_infos($form, $form_state) {
  $skins = skinr_get_skin_info();
  if (empty($skins)) {
    $form['skins_empty'] = array(
      '#markup' => t('You do not currently have any skin sets to manage.'),
    );
    return $form;
  }
  $groups = skinr_get_group_info();

  uasort($skins, 'skinr_ui_sort_by_title');
  $form['skins'] = array('#tree' => TRUE);

  // Iterate through each of the skins.
  foreach ($skins as $name => $skin) {
    $extra = array();

    // Generate link for skin's configuration page
    $extra['links']['configure'] = array(
      '#type' => 'link',
      '#title' => t('Configure'),
      '#href' => 'admin/appearance/skinr/skins/settings/' . $name,
      '#options' => array('attributes' => array('class' => array('skin-link', 'skin-link-configure'))),
    );

    // Create a row entry for this skin.
    $group = $groups[$skin['group']]['title'];
    $form['skins'][$group][$name] = _skinr_ui_admin_skin_infos_build_row($skin, $extra);
  }

  // Add basic information to the fieldsets.
  foreach (element_children($form['skins']) as $package) {
    $form['skins'][$package] += array(
      '#type' => 'fieldset',
      '#title' => $package,
      '#collapsible' => TRUE,
      '#theme' => 'skinr_ui_admin_skin_infos_fieldset',
      '#header' => array(
        t('Name'),
        t('Source type'),
        t('Source name'),
        t('Source version'),
        t('Source plug-in'),
        t('Theme hooks'),
        array('data' => t('Enabled'), 'class' => array('checkbox')),
        array('data' => t('Operations'), 'colspan' => 3),
      ),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['#action'] = url('admin/appearance/skinr/skins');

  return $form;
}

/**
 * Build a table row for the skin info listing page.
 */
function _skinr_ui_admin_skin_infos_build_row($skin, $extra) {
  // Add in the defaults.
  $extra += array(
    'enabled' => FALSE,
    'disabled' => FALSE,
    'links' => array(),
  );
  $form = array(
    '#tree' => TRUE,
  );

  $form['name'] = array(
    '#markup' => $skin['title'],
  );
  $form['description'] = array(
    '#markup' => t($skin['description']),
  );


  // Grab source info.
  $info = system_get_info($skin['source']['type'], $skin['source']['name']);

  $form['source type'] = array(
    '#markup' => $skin['source']['type'],
  );
  $form['source name'] = array(
    '#markup' => $skin['source']['name'],
  );
  $form['source version'] = array(
    '#markup' => $skin['source']['version'],
  );
  $form['source plugin'] = array(
    '#markup' => '', //$skin['source']['plugin'],
  );

  $theme_hooks = array();
  foreach ($skin['theme hooks'] as $theme_hook) {
    $theme_hooks[] = $theme_hook == '*' ? t('all hooks') : $theme_hook;
  }
  $form['theme hooks'] = array(
    '#markup' => implode('<br />', $theme_hooks),
  );

  $form['enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#default_value' => $extra['enabled'],
  );
  if ($extra['disabled']) {
    $form['enable']['#disabled'] = TRUE;
  }

  // Build operation links.
  $form['links']['configure'] = (isset($extra['links']['configure']) ? $extra['links']['configure'] : array());

  return $form;
}

/**
 * Sort skin infos by title.
 *
 * @param $a
 *   The first skin info to compare.
 * @param $b
 *   The second skin info to compare.
 *
 * @return
 *   Returns < 0 if $a is less than $b; > 0 if $a is greater than $b, and 0
 *   if they are equal.
 */
function skinr_ui_sort_by_title($a, $b) {
  return strcasecmp($a['title'], $b['title']);
}

/**
 * Form submission handler for skinr_ui_admin_skin_infos().
 *
 * @todo Update status storage code.
 */
function skinr_ui_admin_skin_infos_submit($form, &$form_state) {
  // Store list of previously enabled themes and disable all themes.
  $old_skin_infos = $new_skin_infos = array();
  foreach (skinr_get_skin_info() as $skin_info) {
    if ($skin_info->status) {
      $old_skin_infos[] = $skin_info->name;
    }
  }
  db_query("UPDATE {skinr_skinsets} SET status = 0");

  // Set status for all
  foreach ($form_state['values']['skinsets'] as $category) {
    foreach ($category as $name => $choice) {
      if ($choice['enable']) {
        $new_theme_list[] = $name;
        db_update('skinr_skinsets')
          ->fields(array('status' => 1))
          ->condition('name', $name)
          ->execute();
      }
    }
  }

  // Refresh skinsets from DB.
  drupal_static_reset('skinr_get_skin_info');

  drupal_set_message(t('The configuration options have been saved.'));

  return;
}

/**
 * Returns HTML for the skin info listing form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_skinr_ui_admin_skin_infos_fieldset($variables) {
  $form = $variables['form'];

  // Individual table headers.
  $rows = array();
  // Iterate through all the modules, which are
  // children of this fieldset.
  foreach (element_children($form) as $skin_name) {
    // Stick it into $skinset for easier accessing.
    $skin_info = $form[$skin_name];

    $row = array();

    // Name.
    $row[] = theme('skinr_ui_admin_skin_infos_summary', array(
      'name' => drupal_render($skin_info['name']),
      'description' => drupal_render($skin_info['description']),
    ));

    // Source.
    $row[] = drupal_render($skin_info['source type']);
    $row[] = drupal_render($skin_info['source name']);
    $row[] = drupal_render($skin_info['source version']);
    $row[] = drupal_render($skin_info['source plugin']);

    // Theme hooks.
    $row[] = drupal_render($skin_info['theme hooks']);

    // Enabled.
    unset($skin_info['enable']['#title']);
    $row[] = array('class' => array('checkbox'), 'data' => drupal_render($skin_info['enable']));

    // Display links (such as help or permissions) in their own columns.
    foreach (array('help', 'permissions', 'configure') as $key) {
      $row[] = array('data' => drupal_render($skin_info['links'][$key]), 'class' => array($key));
    }
    $rows[] = $row;
  }

  return theme('table', array('header' => $form['#header'], 'rows' => $rows));
}

/**
 * Returns HTML for a message about incompatible skinsets.
 *
 * @param $variables
 *   An associative array containing:
 *   - message: The form array representing the currently disabled modules.
 *
 * @ingroup themeable
 */
function theme_skinr_ui_admin_skin_infos_summary($variables) {
  return '<div class="skin-infos-summary"><h2>'. $variables['name'] .'</h2><div class="description">'. $variables['description'] .'</div></div>';
}

/**
 * Returns HTML for a message about incompatible skinsets.
 *
 * @param $variables
 *   An associative array containing:
 *   - message: The form array representing the currently disabled modules.
 *
 * @ingroup themeable
 */
function theme_skinr_ui_admin_skin_infos_incompatible($variables) {
  return '<div class="incompatible">' . $variables['message'] . '</div>';
}

/**
 * Menu callback; displays a listing of all skins in a skinset, allowing you
 * to enable or disable them individually for each theme.
 *
 * @ingroup forms
 * @see skinr_ui_admin_skin_infos_settings_submit()
 */
function skinr_ui_admin_skin_infos_settings($form, &$form_state, $skin_info) {
  $themes = list_themes();
  ksort($themes);

  $form['skins'] = array('#tree' => TRUE);

  foreach ($themes as $theme) {
    if ($theme->status) {
      // Create a row entry for this skinset.
      $form['skins'][$theme->name][$skin_info['name']] = _skinr_ui_admin_skin_infos_settings_build_row($skin_info, $theme->name);
    }
  }

  // Add basic information to the fieldsets.
  $current_theme = skinr_current_theme(TRUE);
  foreach (element_children($form['skins']) as $theme_name) {
    $form['skins'][$theme_name] += array(
      '#type' => 'fieldset',
      '#title' => t($themes[$theme_name]->info['name']),
      '#collapsible' => TRUE,
      '#collapsed' => ($theme_name != $current_theme),
      '#theme' => 'skinr_ui_admin_skin_infos_settings_fieldset',
      '#header' => array(
        t('Enabled'),
        t('Name'),
      ),
    );
  }
  $form_state['skin_info'] = $skin_info;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['actions']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
  );

  return $form;
}

/**
 * Build a table row for the skinset settings page.
 */
function _skinr_ui_admin_skin_infos_settings_build_row($skin, $theme_name) {
  $form['enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#default_value' => (int) !empty($skin['status'][$theme_name]),
  );

  // The hooks this skin is working with.
  $theme_hooks = array();
  foreach ($skin['theme hooks'] as $theme_hook) {
    $theme_hooks[] = $theme_hook == '*' ? t('all hooks') : $theme_hook;
  }
  $features = t('Used by: !theme_hooks', array('!theme_hooks' => implode(', ', $theme_hooks)));

  $form['name'] = array(
    '#markup' => t($skin['title']),
  );
  $form['description'] = array(
    '#markup' => t($skin['description']),
  );
  $form['features'] = array(
    '#markup' => $features,
  );

  return $form;
}

/**
 * Returns HTML for the skinsets settings form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_skinr_ui_admin_skin_infos_settings_fieldset($variables) {
  $form = $variables['form'];

  // Individual table headers.
  $rows = array();
  // Iterate through all the modules, which are
  // children of this fieldset.
  foreach (element_children($form) as $key) {
    // Stick it into $skin for easier accessing.
    $skin = $form[$key];

    $row = array();

    // Enabled.
    unset($skin['enable']['#title']);
    $row[] = array('class' => array('checkbox'), 'data' => drupal_render($skin['enable']));

    // Name.
    $row[] = theme('skinr_ui_admin_skin_infos_settings_summary', array(
      'name' => drupal_render($skin['name']),
      'description' => drupal_render($skin['description']),
      'theme hooks' => drupal_render($skin['theme hooks'])
    ));

    $rows[] = $row;
  }

  return theme('table', array('header' => $form['#header'], 'rows' => $rows));
}

/**
 * Returns HTML for the name column in the skinsets settings form.
 *
 * @param $variables
 *   An associative array containing:
 *   - name
 *   - description
 *   - features
 *
 * @ingroup themeable
 */
function theme_skinr_ui_admin_skin_infos_settings_summary($variables) {
  return '<div class="skin-info"><h2>'. $variables['name'] .'</h2><div class="description">'. $variables['description'] .'</div><div class="theme-hooks">'. $variables['theme hooks'] .'</div></div>';
}

/**
 * Process skinr_ui_admin_skin_infos_settings() form submissions.
 */
function skinr_ui_admin_skin_infos_settings_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Save configuration')) {
    $statuses = array();
    foreach ($form_state['values']['skins'] as $theme => $skins) {
      foreach ($skins as $name => $skin) {
        if ($skin['enable']) {
          $statuses[$name][$theme] = $theme;
        }
      }
    }

    foreach ($statuses as $name => $status) {
      $skinr_skin = new StdClass();
      $skinr_skin->name = $form_state['skin_info']['source']['name'];
      $skinr_skin->type = 'skinset';
      $skinr_skin->skin = $name;
      $skinr_skin->status = $status;

      $exists = (bool) db_query_range('SELECT 1 FROM {skinr_skins} WHERE name = :name AND skin = :skin', 0, 1, array(
        ':name' => $skinr_skin->name,
        ':skin' => $skinr_skin->skin,
      ))->fetchField();
      if ($exists) {
        // Record exists, so let's update.
        drupal_write_record('skinr_skins', $skinr_skin, array('name', 'skin'));
      }
      else {
        // Insert a new record.
        drupal_write_record('skinr_skins', $skinr_skin);
      }
    }
  }
  else {
    // Revert to defaults: disable all skinsets.
    db_delete('skinr_skins')
      ->condition('name', $form_state['values']['skinset'])
      ->execute();
  }

  drupal_set_message(t('The configuration options have been saved.'));
  $form_state['redirect'] = 'admin/appearance/skinr/skins';
}

/**
 * Form builder for the Skinr settings export form.
 *
 * @param $theme
 *   (optional) The name of the theme to export Skinr settings for. If no
 *   theme name is provided a theme selector is shown.
 *
 * @ingroup forms
 */
function skinr_ui_export_form($form, &$form_state, $theme = NULL) {
  $form = array();
  $themes = list_themes();

  if ($theme) {
    // Export an individual theme.
    $theme = str_replace('-', '_', $theme);
    $skins = skinr_skin_load($theme);

    // Convert classes to arrays.
    foreach ($skins as $module => $data) {
      foreach ($data as $sid => $skin) {
        $skins[$module][$sid] = (array) $skins[$module][$sid];
      }
    }

    $code = '$skins = '. var_export($skins, TRUE) .';';
    $lines = substr_count($code, "\n") + 1;

    $form['theme'] = array(
      '#type' => 'textfield',
      '#title' => t('Theme'),
      '#value' => $themes[$theme]->info['name'],
      '#disabled' => TRUE,
    );

    $form['skinr_settings'] = array(
      '#type' => 'textarea',
      '#title' => t('Skinr settings'),
      '#default_value' => $code,
      '#rows' => min($lines, 150),
    );
  }
  else {
    // Give the option for which theme to export.

    $options = array();
    ksort($themes);
    $current_theme = skinr_current_theme(TRUE);

    // Put default theme at the top.
    $options[$current_theme] = $themes[$current_theme]->info['name']. ' [' . t('default') . ']';

    foreach ($themes as $theme) {
      if (!empty($theme->info['hidden'])) {
        continue;
      }

      if ($theme->name == $current_theme) {
        // Do nothing.
      }
      elseif ($theme->status) {
        $options[$theme->name] = $theme->info['name'] . ' [' . t('enabled') . ']';
      }
      else {
        $options[$theme->name] = $theme->info['name'];
      }
    }

    $form['theme'] = array(
      '#type' => 'select',
      '#title' => t('Theme'),
      '#description' => t('Theme to export the skinr settings for.'),
      '#options' => $options,
      '#required' => TRUE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Export'),
    );
  }

  return $form;
}

/**
 * Form validation handler for skinr_ui_export_form().
 */
function skinr_ui_export_form_validate(&$form, &$form_state) {
  if (!empty($form_state['values']['theme']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['theme'])) {
    form_error($form['theme'], t('The theme name must be alphanumeric and can contain underscores only.'));
  }
}

/**
 * Form submission handler for skinr_ui_export_form().
 */
function skinr_ui_export_form_submit(&$form, &$form_state) {
  drupal_goto('admin/appearance/skinr/export/' . str_replace('_', '-', $form_state['values']['theme']));
}

/**
 * Form builder for the Skinr settings import form.
 *
 * @ingroup forms
 */
function skinr_ui_import_form($form, &$form_state) {
  $form = array();

  $options = array();
  $themes = list_themes();
  ksort($themes);
  $current_theme = skinr_current_theme(TRUE);

  // Put default theme at the top.
  $options[$current_theme] = $themes[$current_theme]->info['name']. ' [' . t('default') . ']';

  foreach ($themes as $theme) {
    if (!empty($theme->info['hidden'])) {
      continue;
    }
    if ($theme->name == $current_theme) {
      // Do nothing.
    }
    elseif ($theme->status) {
      $options[$theme->name] = $theme->info['name'] . ' [' . t('enabled') . ']';
    }
    else {
      $options[$theme->name] = $theme->info['name'];
    }
  }

  $form['theme'] = array(
    '#type' => 'select',
    '#title' => t('Theme'),
    '#options' => $options,
    '#required' => TRUE,
  );
  $form['skinr_settings'] = array(
    '#type' => 'textarea',
    '#title' => t('Skinr settings'),
    '#description' => t('Paste skinr settings here.'),
    '#rows' => 16,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

/**
 * Form validation handler for skinr_ui_import_form().
 */
function skinr_ui_import_form_validate(&$form, &$form_state) {
  if (empty($form_state['values']['skinr_settings'])) {
    // Error.
    form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
    return;
  }

  $skins = '';
  ob_start();
  eval($form_state['values']['skinr_settings']);
  ob_end_clean();

  foreach ($skins as $module => $ignored) {
    if (!is_array($skins[$module])) {
      // Error.
      form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
    }
    foreach ($skins[$module] as $sid => $ignored) {
      if (!is_array($skins[$module][$sid])) {
        // Error.
        form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
      }

      // Cast skinr array to object so we can actually use it.
      $skins[$module][$sid] = (object) $skins[$module][$sid];
      if (!skinr_validate($skins[$module][$sid])) {
        // Error.
        form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
      }
    }
  }

  if (!empty($form_state['values']['theme']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['theme'])) {
    form_error($form['theme'], t('The theme name must be alphanumeric and can contain underscores only.'));
  }

  $form_state['skinrs'] = &$skins;
}

/**
 * Form submission handler for skinr_ui_import_form().
 */
function skinr_ui_import_form_submit(&$form, &$form_state) {
  foreach ($form_state['skinrs'] as $module => $skins) {
    foreach ($skins as $sid => $skin) {
      if (!skinr_skin_save($skin)) {
        drupal_set_message(t('Skinr settings for the theme %theme could not be saved!', array('%theme' => $form_state['values']['theme'])), 'error');
      }
    }
  }

  drupal_set_message(t('Skinr settings for the theme %theme have been saved.', array('%theme' => $form_state['values']['theme'])));
  drupal_goto('admin/appearance/skinr');
}

/**
 * Form builder for the skinr settings delete confirmation form.
 *
 * @param $theme
 *   The name of the theme to delete a setting for.
 * @param $module
 *   The module to delete a setting for.
 * @param $sid
 *   The ID of the setting to delete.
 *
 * @ingroup forms
 */
function skinr_ui_delete_confirm($form, &$form_state, $theme, $module, $sid) {
  $form['theme'] = array(
    '#type' => 'value',
    '#value' => isset($form_state['theme']) ? $form_state['theme'] : $theme,
  );
  $form['module'] = array(
    '#type' => 'value',
    '#value' => isset($form_state['module']) ? $form_state['module'] : $module,
  );
  $form['sid'] = array(
    '#type' => 'value',
    '#value' => isset($form_state['sid']) ? $form_state['sid'] : $sid,
  );

  $themes = list_themes();

  return confirm_form($form,
    t('Are you sure you want to delete these Skinr settings?'),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/appearance/skinr',
    t('This action cannot be undone.<br /> Theme: !theme<br />Module: !module<br />Skinr ID: !sid', array('!theme' => $themes[$theme]->info['name'], '!module' => $module, '!sid' => $sid)),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Form submission handler for skinr_ui_delete_confirm().
 */
function skinr_ui_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $skin = new stdClass();
    $skin->theme = $form_state['values']['theme'];
    $skin->module = $form_state['values']['module'];
    $skin->sid = $form_state['values']['sid'];
    $skin->skins = array();
    $skin->settings = array();

    skinr_skin_save($skin);
  }

  $form_state['redirect'] = 'admin/appearance/skinr';
}
