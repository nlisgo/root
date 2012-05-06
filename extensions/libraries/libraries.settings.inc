<?php

/**
 * @file
 * Contains the theme settings form elements for the development extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_libraries_theme_settings_form_alter($element, &$form, $form_state) {
  $libraries = theme_get_setting('root_libraries');
  $trail = root_theme_trail();

  foreach (root_theme_libraries_info() as $library => $info) {
    if (isset($info['theme']) && array_key_exists($info['theme'], $trail)) {
      $element[$library]['status'] = array(
        '#type' => 'checkbox',
        '#title' => isset($info['vendor url']) ? t('@library by <a href="!url">@vendor</a>', array('@library' => $info['name'], '!url' => $info['vendor url'], '@vendor' => $info['vendor'])) : t('@library by @vendor', array('@library' => $info['name'], '@vendor' => $info['vendor'])),
        '#description' => $info['description'],
        '#default_value' => !empty($libraries[$library]['status']),
        '#parents' => array('root_libraries', $library, 'status'),
      );

      $options = array();
      if (!empty($info['variants'])) {
        $variants = array();
        foreach ($info['variants'] as $variant => $variant_info) {
          $variants[$variant] = $variant_info['name'] . (isset($variant_info['description']) ? '<div class="description">' . $variant_info['description'] . '</div>': '');
        }

        if (!empty($variants)) {
          $variants = array_merge(array('_default' => t('Default') . '<div class="description">' . t('The default variant generally is a minified version of the library and provides the optimal production solution in most cases.') . '</div>'), $variants);

          $options['variant'] = array(
            '#type' => 'radios',
            '#title' => t('Variant'),
            '#options' => $variants,
            '#default_value' => !empty($libraries[$library]['variant']) ? $libraries[$library]['variant'] : '_default',
            '#parents' => array('root_libraries', $library, 'variant'),
          );
        }
      }

      // Some libraries might want to provide special options.
      if (isset($info['options form callback']) && function_exists($info['options form callback'])) {
        // If a library options form callback exists and actually returns some
        // options for this library append conditionally show them underneath
        // the library checkbox with #states.
        $info['options form callback']($options, $form, $form_state, $library, $info);
      }

      if (!empty($options)) {
        $element[$library]['options'] = array(
          '#type' => 'container',
          '#states' => array(
            'visible' => array(
              ':input[name="root_libraries[' . $library . '][status]"]' => array('checked' => TRUE),
            ),
          ),
          '#parents' => array('root_libraries', $library),
          '#tree' => TRUE,
          '#attributes' => array('class' => array('root-settings-container')),
        ) + $options;
      }
    }
  }

  $form['#submit'][] = 'root_extension_libraries_theme_settings_form_submit';

  return $element;
}

/**
 * Theme settings form submit handler for the libraries extension.
 */
function root_extension_libraries_theme_settings_form_submit($form, &$form_state) {
  if (!empty($form_state['values']['root_libraries'])) {
    $values = &$form_state['values']['root_libraries'];

    foreach ($values as &$settings) {
      if (isset($settings['variant']) && $settings['variant'] == '_default') {
        // Unset the 'variant' option if it is the default option.
        unset($settings['variant']);
      }
    }
  }
}

/**
 * Theme settings form submit handler for handling the PIE selectors.
 */
function root_library_pie_selectors_submit($form, &$form_state) {
  $values = &$form_state['values']['root_libraries']['css3pie'];

  // The generated CSS file is stored in the public file system.
  $theme = $form_state['build_info']['args'][0];
  $destination = 'public://root/' . $theme;

  // Generate a CSS file for the provided selectors if the inclusion method is
  // set to 'pie.htc' or 'pie.php'.
  if (!empty($values['selectors'])) {
    // Sanitize the selectors.
    $selectors = array_map('trim', explode("\n", $values['selectors']));
    $selectors = array_unique($selectors);

    // Retrieve the path to the library.
    $path = base_path() . libraries_get_path('css3pie') . '/pie.htc';

    // Build the contents of the CSS and JS file file.
    $css = implode(",", $selectors) . "{behavior:url($path)}";
    $js = '$(function(){Drupal.behaviors.css3pie={attach:function(context,settings){if(window.PIE){$("' . implode(",", $selectors) . '").each(function(){PIE.attach(this)})}}}})(jQuery);';

    // Save the generated CSS in the public file system replacing any previous
    // version.
    file_prepare_directory($destination, FILE_CREATE_DIRECTORY);
    file_unmanaged_save_data($css, $destination . '/pie-selectors.css', FILE_EXISTS_REPLACE);
    file_unmanaged_save_data($js, $destination . '/pie-selectors.js', FILE_EXISTS_REPLACE);
  }
  else {
    // Otherwise, we can delete the (now stale) CSS file.
    file_unmanaged_delete($destination . '/pie-selectors.css');
  }

  // Set the selectors options to TRUE or FALSE depending on whether there are
  // selectors to be loaded or not.
  $parents = array('#parents' => array('root_libraries', 'css3pie', 'selectors'));
  form_set_value($parents, !empty($values['selectors']), $form_state);
}


/**
 * CSS3PIE Library options form callback.
 */
function root_library_pie_options_form(&$element, &$form, $form_state, $library, $info) {
  // Give the default variant a proper label.
  $element['variant']['#options']['_default'] = t('HTML Component (default)');

  // Pull the theme key from the form arguments.
  $theme = $form_state['build_info']['args'][0];
  // Load the contents of the current version of the PIE selector css file.
  $source = 'public://root/' . $theme . '/pie-selectors.css';

  // Load the current PIE selectors file if one exists.
  if ($selectors = @file_get_contents($source)) {
    // We need to sanitize the output for our textarea.
    $selectors = preg_replace('/\{behavior:url\((.*)\)\}/', '', $selectors);
    $selectors = array_filter(array_map('trim', explode(',', $selectors)));
    $selectors = implode("\n", $selectors);
  }

  $element['selectors'] = array(
    '#title' => t('Selectors'),
    '#type' => 'textarea',
    '#description' => t("You can use this textarea to define all the CSS rules that you want to apply the PIE behavior to. Define one CSS selector per line. Note: The value of this field is not stored as a theme settings as it is directly written to a .css file in the <a href=\"!url\">public file system</a> to not clutter the theme settings array. Therefore, it won't get exported if you choose to export your theme settings.", array('!url' => file_create_url($source))),
    '#default_value' => $selectors ? $selectors : '',
  );

  // We need to provide a submit handler to create a CSS file for the
  // defined selectors and remove them from the theme settings array.
  $form['#submit'][] = 'root_library_pie_selectors_submit';
}
