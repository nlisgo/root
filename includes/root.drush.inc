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
    'description' => dt('Creates a Root subtheme.'),
    'arguments' => array(
      'name' => dt('The name of your subtheme.'),
      'machine_name' => dt('[optional] The machine-readable name of your subtheme. This will be auto-generated from the human-readable name if omitted.'),
    ),
    'options' => array(
      'destination' => dt('The destination of your subtheme. Defaults to "all" (sites/all/themes).'),
      'machine_name' => dt('The machine-readable name of your subtheme. This will be auto-generated from the human-readable name if omitted.'),
      'starterkit' => dt('The starterkit that your subtheme should use. Defaults to "root_starterkit".'),
      'enable' => dt('Automatically enable the subtheme after creation.'),
      'set-default' => dt('Automatically enable the subtheme after creation and make it the default theme.'),
    ),
    'examples' => array(
      'drush root-subtheme "My Theme"' => dt('Creates a Root subtheme called "My Theme".'),
      'drush root-subtheme "My Theme" --destination=example.com' => dt('Creates a Root subtheme called "My Theme" in sites/example.com/themes.'),
      'drush root-subtheme "My Theme" --starterkit=my_custom_starterkit' => dt('Uses a custom starterkit to create a Root subtheme called "My Theme" in sites/all/themes (default).'),
    ),
    'aliases' => array('rsub'),
  );

  $items['root-export'] = array(
    'description' => dt('Exports the theme settings of a given theme from the database to the .info file.'),
    'arguments' => array(
      'theme' => dt('The machine-readable name of the theme to export the theme settings for.'),
    ),
    'options' => array(
      'revert' => dt('Purges the theme settings from the database after exporting them to the .info file.'),
    ),
    'examples' => array(
      'drush root-export foo' => dt('Exports the theme settings of the "foo" theme to the "foo.info" file in that theme.'),
      'drush root-export foo --revert' => dt('Purges the theme settings of the "foo" theme from the database after exporting them to the .info file.'),
    ),
    'aliases' => array('rexp'),
  );

  $items['root-revert'] = array(
    'description' => dt('Reverts the theme settings of a given theme by deleting them from the database.'),
    'arguments' => array(
      'theme' => dt('The machine-readable name of the theme to revert the theme settings for.'),
    ),
    'examples' => array(
      'drush root-revert foo' => dt('Reverts the theme settings of the "foo" theme.'),
    ),
    'aliases' => array('rrev'),
  );

  $items['root-download-libraries'] = array(
    'description' => dt('Downloads all available (and downloadable) libraries defined by the current default theme and all its base themes.'),
    'aliases' => array('rdll'),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function root_drush_help($section) {
  switch ($section) {
    case 'drush:root-subtheme':
      return dt('Generates a subtheme.');
    case 'drush:root-export':
      return dt('Exports the theme settings of a given theme.');
    case 'drush:root-revert':
      return dt('Reverts the theme settings of a given theme.');
    case 'drush:root-download-libraries':
      return dt('Downloads all available (and downloadable) libraries for a theme.');
  }
}

/**
 * Implements drush_hook_COMMAND_validate().
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
    return drush_set_error('ROOT_THEME_NAME_INVALID', dt('The machine name is invalid. It may only contain lowercase numbers, letters and underscores and must start with a letter.'));
  }

  // Validate that the machine-readable name of the theme is unique.
  if (isset($themes[$machine_name])) {
    return drush_set_error('ROOT_THEME_ALREADY_EXISTS', dt('A theme with that name already exists. The machine-readable name must be unique.'));
  }

  // Check if the destination is valid.
  if (!in_array($destination, root_generator_sites())) {
    return drush_set_error('ROOT_DESITINATION_INVALID', dt('The destination is invalid.'));
  }

  // Check if the chosen base theme or starterkit exists.
  if (!array_key_exists($starterkit, root_generator_starterkits())) {
    return drush_set_error('ROOT_STARTERKIT_NOT_EXISTS', dt('There is no valid Root theme starterkit with the name @starterkit.', array('@starterkit' => $starterkit)));
  }
}

/**
 * Implements drush_hook_COMMAND().
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
    drush_log(dt('You have successfully created the theme @theme.', array('@theme' => $subtheme->name)), 'success');
  }
  else {
    drush_set_error('ROOT_SUBTHEME_CREAT_ERROR', dt('There was an error when creating the theme @theme. Please refer to watchdog for further information.', array('@theme' => $subtheme->name)), 'error');
  }
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_root_export_validate($theme) {
  $themes = list_themes();
  // Check if the given theme exists.
  if (!isset($themes[$theme])) {
    return drush_set_error('ROOT_THEME_NOT_EXISTS', dt('There is no theme with that name.'));
  }
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_root_export($theme) {
  $themes = list_themes();

  if (root_generator_export_theme_settings($theme)) {
    drush_log(dt('The theme settings for the @theme theme have been successfully exported.', array('@theme' => $themes[$theme]->info['name'])), 'success');

    if (drush_get_option('revert')) {
      // Revert the theme settings if the 'revert' option is set and they have
      // been exported successfully. In this case, we invoke the API function
      // through the drush command to get the  print messages displazed.
      drush_op('drush_root_revert', $theme);
    }
  }
  else {
    // There was an error while exporting the theme settings.
    drush_set_error('ROOT_EXPORT_ERROR', dt('An error occurred while trying to export the theme settings for the @theme theme.', array('@theme' => $themes[$theme]->info['name'])));
  }
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_root_revert_validate($theme) {
  $themes = list_themes();
  // Check if the given theme exists.
  if (!isset($themes[$theme])) {
    return drush_set_error('ROOT_THEME_NOT_EXISTS', dt('There is no theme with that name.'));
  }
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_root_revert($theme) {
  $themes = list_themes();
  root_generator_revert_theme_settings($theme);
  drush_log(dt('You have successfully reverted the theme settings for the @theme theme.', array('@theme' => $themes[$theme]->info['name'])), 'success');
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_root_download_libraries_validate() {
  // @todo change the array index to '0' once we are no longer a bartik subtheme.
  if (!isset($GLOBALS['base_theme_info'][1]) || $GLOBALS['base_theme_info'][1]->name != 'root') {
    // If this is not 'root' the current default theme is not a Root subtheme.
    return drush_set_error('ROOT_ERROR', dt('The current default theme has to be a Root subtheme for this operation to work.'));
  }
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_root_download_libraries() {
  foreach (root_invoke_all('root_libraries_info') as $library => $info) {
    if (!empty($info['tarball download']) || !empty($info['direct download'])) {
      $path = module_exists('libraries') ? libraries_get_path($library) : drupal_get_path('theme', $GLOBALS['theme_key']) . '/libraries';

      if (!is_dir($path) && !drush_mkdir($path)) {
        // Jump to the next file if the directory doesn't exists and there was
        // a problem when creating it.
        continue;
      }

      if (!is_writable($path)) {
        // Continue with the next file if the directory is not writable.
        continue;
      }

      // Process all tarball downloads and extractions if any.
      if (!empty($info['tarball download'])) {
        // We use an arbitary filename, since some sources (e.g. github) do not
        // include a filename in the URL.
        $tarball = $path . '/drush-library-' . $library . '.tar.gz';

        // Download and extract the tarball to the library folder.
        if (drush_download_file($info['tarball download'], $tarball) && drush_tarball_extract($tarball)) {
          // The download succeeded.
          drush_log(dt('The tarball for the @label library has been downloaded and extracted in @path.', array('@label' => $info['label'], '@path' => $path)), 'success');
        }
        else {
          // There was a problem.
          drush_set_error('ROOT_LIBRARY_DOWNLOAD_ERROR', dt('There was a problem when trying to download the tarball for the @label library.', array('@label' => $info['label'])));
        }
      }

      // Process all direct file downloads if any.
      if (!empty($info['direct download'])) {
        foreach ($info['direct download'] as $source => $filename) {

          if (file_exists($path . '/' . $filename) && !drush_confirm(dt('There is already a @file file in @path. Do you want to override it?', array('@file' => $filename, '@path' => $path)))) {
            // Don't download the file if it already exists and the user doesn't
            // want to override it.
            continue;
          }

          // Download the file to the libraries folder.
          if (_drush_download_file($source, $path . '/' . $filename)) {
            // The download succeeded.
            drush_log(dt('The @file file for the @label library has been downloaded to @path.', array('@file' => $filename, '@label' => $info['label'], '@path' => $path)), 'success');
          }
          else {
            // There was a problem.
            drush_set_error('ROOT_LIBRARY_DOWNLOAD_ERROR', dt('There was a problem when trying to download the @file file for the @label library.', array('@file' => $filename, '@label' => $info['label'])));
          }
        }
      }
    }
  }
}
