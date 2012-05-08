(function ($) {

/**
 * Renders a widget for displaying the viewport properties at the bottom of the
 * screen.
 */
Drupal.behaviors.rootViewportWidget = {
  attach: function (context, settings) {
    $('body').once('root-viewport-widget', function (){
      $wrapper = $('<div class="root-viewport-widget-wrapper" />').appendTo(this);
      $widget = $('<div class="root-viewport-widget" />').appendTo($wrapper);
      $close = $('<a class="root-viewport-widget-close" title="' + Drupal.t('Close') + '" />').appendTo($widget).click(function () {
        $wrapper.removeClass('root-viewport-widget-toggled');
      });
      $open = $('<a class="root-viewport-widget-open" title="' + Drupal.t('Open') + '" />').insertAfter($widget).click(function () {
        $wrapper.addClass('root-viewport-widget-toggled');
      });

      $width = $('<div class="root-viewport-widget-width"><span class="root-viewport-widget-property" /></div>').appendTo($widget);
      $height = $('<div class="root-viewport-widget-height"><span class="root-viewport-widget-property" /></div>').appendTo($widget);

      $(window).bind('resize.root-viewport-widget', function () {
        $width.find('span.root-viewport-widget-property').text($(window).width() + 'px');
        $height.find('span.root-viewport-widget-property').text($(window).height() + 'px');
      }).trigger('resize.root-viewport-widget');
    });
  }
}

})(jQuery);
