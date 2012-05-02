<?php

/**
 * @file
 * Please supply a file description.
 */

require_once dirname(__FILE__) . '/root.generator.inc';

/**
 * Implements hook_drush_command().
 */
function root_drush_command() {
  $items['root-subtheme'] = array(
    'description' => 'Create a Root subtheme.',
    'arguments' => array(
      'name' => 'The name of your subtheme.',
      'machine_name' => '[optional] The machine-readable name of your subtheme. This will be auto-generated from the human-readable name if ommited.',
    ),
    'options' => array(
      'destination' => 'The destination of your subtheme. Defaults to "all" (sites/all/themes).',
      'machine_name' => 'The machine-readable name of your subtheme. This will be auto-generated from the human-readable name if ommited.',
      'starterkit' => 'The starterkit that your subtheme should use. Defaults to "root_starterkit".',
      'enable' => 'Automatically enable the subtheme after creation.',
      'set-default' => 'Automatically enable the subtheme after creation and make it the default theme.',
    ),
    'examples' => array(
      'drush root-subtheme "My Theme"' => 'Creates a Root subtheme called "My Theme".',
      'drush root-subtheme "My Theme" --destination=example.com' => 'Creates a Root subtheme called "My Theme" in sites/example.com/themes.',
      'drush root-subtheme "My Theme" --starterkit=my_custom_starterkit' => 'Uses a custom starterkit to create a Root subtheme called "My Theme" in sites/all/themes (default).',
    ),
    'aliases' => array('rsub'),
  );

  $items['root-export'] = array(
    'description' => 'Exports the theme settings of a given theme from the database to the .info file.',
    'arguments' => array(
      'theme' => 'The machine-readable name of the theme to export the theme settings for.',
    ),
    'options' => array(
      '--revert' => 'Purges the theme settings from the database after exporting them to the .info file.',
    ),
    'examples' => array(
      'drush root-export foo' => 'Exports the theme settings of the "foo" theme to the "foo.info" file in that theme.',
      'drush root-export foo --revert' => 'Purges the theme settings of the "foo" theme from the database after exporting them to the .info file.',
    ),
    'aliases' => array('rexp'),
  );

  $items['root-revert'] = array(
    'description' => 'Reverts the theme settings of a given theme by deleting them from the database.',
    'arguments' => array(
      'theme' => 'The machine-readable name of the theme to revert the theme settings for.',
    ),
    'examples' => array(
      'drush root-revert foo' => 'Reverts the theme settings of the "foo" theme.',
    ),
    'aliases' => array('rrev'),
  );

  return $items;
}

/**
 * Implements of hook_drush_help().
 */
function root_drush_help($section) {
  switch ($section) {
    case 'drush:root-subtheme':
      return dt('Create a subtheme from starterkit.');
    case 'drush:root-export':
      return dt('Exports the theme settings of a given theme to the .info file of that theme.');
    case 'drush:root-revert':
      return dt('Reverts the theme settings of a given theme.');
  }
}

/**
 * Implements of drush_hook_COMMAND_validate().
 */
function drush_root_subtheme_validate($name, $machine_name = NULL) {
  // Rebuild the theme data so that we can safely check for the existance of
  // themes by using the information provided by list_themes().
  system_rebuild_theme_data();

  $themes = list_themes();

  // Retrieve the option values from the drush command.
  $destination = drush_get_option('destination', 'all');
  $machine_name = $machine_name ? $machine_name : drush_get_option('machine_name', _root_generator_generate_theme_name($name));
  $starterkit = drush_get_option('starterkit', 'root_starterkit');

  // Override the machine-readable name of the theme in case it was generated
  // from the human-readable name.
  drush_set_option('machine_name', $machine_name);

  // Validate the machine-readable name of the theme.
  if (!$machine_name || !preg_match('/^[a-z][a-z0-9_]*$/', $machine_name)) {
    return drush_set_error('ROOT_GENERATOR_ERROR', dt('The machine name is invalid. It may only contain lowercase numbers, letters and underscores and must start with a letter.'));
  }

  // Validate that the machine-readable name of the theme is unique.
  if (isset($themes[$machine_name])) {
    return drush_set_error('ROOT_GENERATOR_ERROR', dt('A theme with that name already exists. The machine-readable name must be unique.'));
  }

  // Check if the destination is valid.
  if (!in_array($destination, root_generator_sites())) {
    return drush_set_error('ROOT_GENERATOR_ERROR', dt('The destination is invalid.'));
  }

  // Check if the chosen base theme or starterkit exists.
  if (!array_key_exists($starterkit, root_generator_starterkits())) {
    return drush_set_error('ROOT_GENERATOR_ERROR', dt('There is no valid Root theme starterkit with the name @starterkit.', array('@starterkit' => $starterkit)));
  }
}

/**
 * Implements of drush_hook_COMMAND().
 */
function drush_root_subtheme($name) {
  $subtheme = new stdClass();
  $subtheme->name = $name;
  $subtheme->machine_name = drush_get_option('machine_name');
  $subtheme->path = 'sites/' . drush_get_option('destination', 'all') . '/themes/' . $subtheme->machine_name;
  $subtheme->starterkit = drush_get_option('starterkit', 'root_starterkit');
  $subtheme->default = drush_get_option('set-default') !== NULL;
  $subtheme->enable = $subtheme->default || drush_get_option('enable') !== NULL;

  if (root_generator_subtheme_create($subtheme)) {
    drush_print(dt('You have successfully created the theme @theme.', array('@theme' => $subtheme->name)));
  }
  else {
    drush_print(dt('There was an error when creating the theme @theme. Please refer to watchdog for further information.', array('@theme' => $subtheme->name)));
  }
}

/**
 * Implements of drush_hook_COMMAND_validate().
 */
function drush_root_export_validate($theme) {
  $themes = list_themes();
  // Check if the given theme exists.
  return isset($themes[$theme]);
}

/**
 * Implements of drush_hook_COMMAND().
 */
function drush_root_export($theme) {
  $themes = list_themes();

  if (isset($themes[$theme])) {
    if (root_generator_export_theme_settings($theme)) {
      drush_print(dt('The theme setings for the @theme theme have been successfully exported.', array('@theme' => $themes[$theme]->info['name'])));

      if (drush_get_option('revert')) {
        // Revert the theme settings if the '--revert' option is set and they have
        // been exported successfully.
        root_generator_revert_theme_settings($theme);
      }
    }
    else {
      // There was an error while exporting the theme settings.
      drush_print(dt('An error occured while trying to export the theme settings for the @theme theme.', array('@theme' => $themes[$theme]->info['name'])));
    }
  }
  else {
    // The theme doesn't exist.
    drush_print(dt("The theme that that you tried to export the theme settings for doesn't exist."));
  }
}

/**
 * Implements of drush_hook_COMMAND_validate().
 */
function drush_root_revert_validate($theme) {
  $themes = list_themes();
  // Check if the given theme exists.
  return isset($themes[$theme]);
}

/**
 * Implements of drush_hook_COMMAND().
 */
function drush_root_revert($theme) {
  $themes = list_themes();

  if (isset($themes[$theme])) {
    root_generator_revert_theme_settings($theme);
    drush_print(dt('You have successfully reverted the theme settings for the @theme theme.', array('@theme' => $themes[$theme]->info['name'])));
  }
  else {
    // The theme doesn't exist.
    drush_print(dt("The theme that that you tried to revert the theme settings for doesn't exist."));
  }
}