<?php

/**
 * @file
 * Theme settings file for the Root base theme.
 */

require_once dirname(__FILE__) . '/template.php';

/**
 * Implements hook_form_FORM_alter().
 */
function root_form_system_theme_settings_alter(&$form, $form_state) {
  if ($form_state['build_info']['args'][0] == $GLOBALS['theme_key']) {
    // Add some custom styling to our theme settings form.
    $form['#attached']['css'][] = drupal_get_path('theme', 'root') . '/css/root.admin.css';

    // Collapse all the core theme settings tabs in order to have the form actions
    // visible all the time without having to scroll.
    foreach (element_children($form) as $key) {
      if ($form[$key]['#type'] == 'fieldset')  {
        $form[$key]['#collapsible'] = TRUE;
        $form[$key]['#collapsed'] = TRUE;
      }
    }

    $form['root'] = array(
      '#type' => 'vertical_tabs',
      '#weight' => -10,
    );

    // Load the theme settings for all enabled extensions.
    foreach (root_extensions() as $extension) {
      // Load all the implementations for this extensions and invoke the according
      // hooks.
      root_theme_trail_load_include('inc', 'extensions/' . $extension . '/' . $extension . '.settings');

      foreach (root_theme_trail() as $theme => $title) {
        $function = $theme . '_extension_' . $extension . '_theme_settings_form_alter';
        if (function_exists($function)) {
          // By default, each extension resides in a vertical tab.
          $element = array(
            '#type' => 'fieldset',
            '#title' => t(filter_xss_admin(ucfirst($extension))),
          );

          $form['root']['root_' . $extension] = $function($element, $form, $form_state);
        }
      }
    }

    // We need a custom form submit handler for processing some of the values.
    $form['#submit'][] = 'root_theme_settings_form_submit';
  }
}

/**
 * Form submit handler for the theme settings form.
 */
function root_theme_settings_form_submit($form, &$form_state) {
  // Clear the theme settings cache.
  cache_clear_all('theme_settings:' . $form_state['build_info']['args'][0], 'cache');

  // This is a relict from the vertical tabs and should be removed so it doesn't
  // end up in the theme settings array.
  unset($form_state['values']['root__active_tab']);
}
