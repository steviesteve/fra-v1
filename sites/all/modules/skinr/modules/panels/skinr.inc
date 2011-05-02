<?php

/**
 * @file
 * Definition of the 'skinr' panel style.
 */

// Plugin definition
$plugin = array(
  'title'              => t('Skinr'),
  'description'        => t('Allows you to apply Skinr skins.'),
  'render region'      => 'panels_skinr_style_render_region',
  'render pane'        => 'panels_skinr_style_render_pane',
  'settings form'      => 'panels_skinr_style_settings_form',
  'pane settings form' => 'panels_skinr_style_settings_form',
);

/**
 * Render region callback.
 *
 * @ingroup themeable
 */
function theme_panels_skinr_style_render_region($variables) {
  $hook = 'panels';
  $sid = 'display-' . $variables['display']->did . '-region-' . $variables['region_id'];

  // Add skinr styles.
  $style = array();
  if ($skinr = skinr_skin_load(NULL, $hook, $sid)) {
    $style = implode(' ', skinr_flatten_skins_array($skinr->skins));
  }

  $output = implode('<div class="panel-separator"></div>', $variables['panes']);
  if (!empty($style)) {
    $output = '<div class="' . $style . '">' . $output . '</div>';
  }

  // Add our contextual links here because panels does some crazy custom stuff.
  if (user_access('skinr access')) {
    $variables['content']->admin_links[] = array(
      'title' => t('Edit skin'),
      'href' => 'admin/appearance/skinr/edit/nojs/panels/' . $sid,
      'query' => drupal_get_destination(),
    );
  }

  return $output;
}

/**
 * Render pane callback.
 *
 * @ingroup themeable
 */
function theme_panels_skinr_style_render_pane($variables) {
  $hook = 'panels';
  $sid = 'display-' . $variables['display']->did . '-pane-' . $variables['pane']->pid;

  // Add skinr styles
  $style = array();
  if ($skinr = skinr_skin_load(NULL, $hook, $sid)) {
    $style = skinr_flatten_skins_array($skinr->skins);
  }

  if (!empty($style)) {
    if (!empty($variables['classes_array'])) {
      $variables['classes_array'] += $style;
    }
    else {
      $variables['classes_array'] = $style;
    }
  }

  // Add our contextual links here because panels does some crazy custom stuff.
  if (user_access('skinr access')) {
    $variables['content']->admin_links[] = array(
      'title' => t('Edit skin'),
      'href' => 'admin/appearance/skinr/edit/nojs/panels/' . $sid,
      'query' => drupal_get_destination(),
    );
  }

  return theme('panels_pane', $variables);
}

/**
 * Settings form callback.
 */
function panels_skinr_style_settings_form($style_settings) {
  // Return an empty form, and let skinr handle the rest.
  $form = array();
  return $form;
}
