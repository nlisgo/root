<?php

/**
 * @file
 * Contains the theme settings form elements for the development extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_libraries_theme_settings_form_alter($element, &$form, $form_state) {
  $element['#description'] = t("Remember to download all the libraries that you want to use to your theme directory as we won't include any of them in the base theme to prevent them from becoming out-dated.");

  $libraries = array();
  foreach (root_invoke_all('root_libraries_info') as $library => $info) {
    $libraries['root_library_' . $library] = array(
      '#type' => 'checkbox',
      '#title' => $info['label'],
      '#description' => $info['description'],
      '#default_value' => theme_get_setting('root_library_' . $library),
    );

    // Some libraries might want to provide special options.
    $function = $info['theme'] . '_library_options_form';
    if (function_exists($function)) {
      $options = array();
      // If a library options form callback exists and actually returns some
      // options for this library append conditionally show them underneath the
      // library checkbox with #states.
      if ($options = $function($options, $form, $form_state, $library, $info)) {
        $libraries['root_library_' . $library . '_options'] = array(
          '#type' => 'container',
          '#states' => array(
            'visible' => array(
              ':input[name="root_library_' . $library . '"]' => array('checked' => TRUE),
            ),
          ),
          '#attributes' => array('class' => array('root-settings-container')),
        ) + $options;
      }
    }
  }

  if (!empty($libraries)) {
    $element['root_minified_libraries'] = array(
      '#type' => 'checkbox',
      '#title' => t('Preferably include minified versions of selected libraries'),
      '#description' => t('Loads the minified (production) version instead of the un-minified (development) version of a library. Highly suggested when going into production.'),
      '#default_value' => theme_get_setting('root_minified_libraries'),
      '#weight' => -10,
    );

    // Place the available library options in a dedicated fieldset.
    $element['root_libraries'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available libraries'),
    ) + $libraries;
  }

  return $element;
}

/**
 * Theme settings form submit handler for handling the PIE selectors.
 */
function root_library_pie_selectors_submit($form, &$form_state) {
  $values = $form_state['values'];

  // Pull the theme key from the form arguments.
  $theme = $form_state['build_info']['args'][0];

  // The generated file is placed in the public file system and includes the
  // name of the theme.
  $destination = 'public://root/' . $theme . '-pie-selectors.css';

  // Generate a CSS file for the provided selectors if the inclusion method is
  // set to 'pie.htc' or 'pie.php'.
  if ($values['root_library_pie_selectors'] && in_array($values['root_library_pie_inclusion'], array('pie.htc', 'pie.php'))) {
    // Sanitize the selectors.
    $selectors = array_map('trim', explode("\n", $values['root_library_pie_selectors']));
    $selectors = array_unique($selectors);

    // Retrieve the path to the library (use the library module if it is enabled
    // or fall back to the path of the theme).
    if (module_exists('libraries')) {
      $path = libraries_get_path('pie') . '/' . $values['root_library_pie_inclusion'];
    }
    else {
      $path = drupal_get_path('theme', $theme) . '/libraries/pie/' . $values['root_library_pie_inclusion'];
    }

    // Build the contents of the CSS file.
    $data = implode(",\n", $selectors) . " { behavior: url($path); }";
    // Save the generated CSS in the public file system replacing any previous
    // version.
    file_prepare_directory(dirname($destination), FILE_CREATE_DIRECTORY);
    file_unmanaged_save_data($data, $destination, FILE_EXISTS_REPLACE);
  }
  else {
    // Otherwise, we can delete the (now stale) CSS file.
    file_unmanaged_delete($destination);
  }

  // Purge the selectors from the theme settings array.
  unset($form_state['values']['root_library_pie_selectors']);
}
