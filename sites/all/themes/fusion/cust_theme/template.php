<?php

function cust_theme_preprocess_html(&$variables) {
  $options = array(
    'group' => JS_THEME,
    'type' => 'file'
  );
  drupal_add_js(drupal_get_path('theme', 'cust_theme'). '/js/scroll.js', $options);
	drupal_add_js(drupal_get_path('theme', 'cust_theme'). '/js/tinyscrollbar.js', $options);
}

function cust_theme_preprocess_page(&$vars,$hook) {
  // Set grid width
  $grid = fusion_core_grid_info();
  $vars['grid_width'] = $grid['name'] . $grid['width'];

  // Adjust width variabl	es for nested grid groups
  $grid_adjusted_groups = (theme_get_setting('grid_adjusted_groups')) ? theme_get_setting('grid_adjusted_groups') : array();
  foreach (array_keys($grid_adjusted_groups) as $group) {
    $width = $grid['width'];
    foreach ($grid_adjusted_groups[$group] as $region) {
      $width = $width - $grid['regions'][$region]['width'];
    }
    if (!$grid['fixed'] && isset($grid['fluid_adjustments'][$group])) {
      $vars[$group . '_width'] = '" style="width:' . $grid['fluid_adjustments'][$group] . '%"';
    }
    else {
      $vars[$group . '_width'] = $grid['name'] . $width;
    }
  }

  // Remove breadcrumbs if disabled
  if (theme_get_setting('breadcrumb_display') == 0) {
    $vars['breadcrumb'] = '';
  }

if (isset($vars['node'])) { 
    $vars['theme_hook_suggestions'][] = 'page__type__'. $vars['node']->type;
    $vars['theme_hook_suggestions'][] = "page__node__" . $vars['node']->nid;

	}
}


 /**
 * Block preprocessing
 */
function cust_theme_preprocess_block(&$vars) {
  global $theme_info, $user;
  static $grid;

  // Exit if block is outside a defined region
  if (!in_array($vars['block']->region, array_keys($theme_info->info['regions']))) {
    return;
  }

  // Initialize grid info once per page
  if (!isset($grid)) {
    $grid = fusion_core_grid_info();
  }

  // Increment block count for current block's region, add first/last position class
  $grid['regions'][$vars['block']->region]['count'] ++;
  $region_count = $grid['regions'][$vars['block']->region]['count'];
  $total_blocks = $grid['regions'][$vars['block']->region]['total'];
  $vars['classes_array'][] = ($region_count == 1) ? 'first' : '';
  $vars['classes_array'][] = ($region_count == $total_blocks) ? 'last' : '';
  $vars['classes_array'][] = $vars['block_zebra'];

  // Set a default block width if not already set by Skinr
  if($vars['block']->delta == 'main-menu') {
	$count=0;
	foreach($vars['classes_array'] as $key=>$class) {
		if (strpos($class, $grid['name']) !== false) {
			array_splice($vars['classes_array'],$count,1);
		}
	$count++;
	}
	$vars['classes_array'][] = $grid['name'] . 3;
  }

  $classes = implode(' ', $vars['classes_array']);
  if (strpos($classes, $grid['name']) === false) {
    // Stack blocks vertically in sidebars by setting to full sidebar width
    if ($vars['block']->region == 'sidebar_first') {
      $width = $grid['fixed'] ? $grid['sidebar_first_width'] : $grid['width'];  // Sidebar width or 100% (if fluid)
    }
    elseif ($vars['block']->region == 'sidebar_second') {
      $width = $grid['fixed'] ? $grid['sidebar_second_width'] : $grid['width'];  // Sidebar width or 100% (if fluid)
    }
    else {
      // Default block width = region width divided by total blocks, adding any extra width to last block
      $region_width = ($grid['fixed']) ? $grid['regions'][$vars['block']->region]['width'] : $grid['width'];  // fluid grid regions = 100%
      $width_adjust = (($region_count == $total_blocks) && ($region_width % $total_blocks)) ? $region_width % $total_blocks : 0;
      $width = ($total_blocks) ? floor($region_width / $total_blocks) + $width_adjust : 0;
    }
    $vars['classes_array'][] = $grid['name'] . $width;
  }
}

?>

