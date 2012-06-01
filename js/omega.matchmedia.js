(function ($) {

  /**
   * Container for all Omega related objects.
   */
  Drupal.omega = Drupal.omega || {};

  /**
   * Container object for the media queries that should be available to
   * JavaScript.
   */
  Drupal.omega.mediaQueries = Drupal.omega.mediaQueries || {};

  /**
   * Provides a lightweight resize event that only fires once at the end of a
   * browser resize.
   *
   * @param el
   *   The element to attach the event to.
   * @param options
   *   The options for the resizeend event.
   */
  $.resizeend = function (el, options){
    var base = this;

    base.$el = $(el);
    base.el = el;

    base.$el.data("resizeend", base);
    base.rtime = new Date(1, 1, 2000, 12,00,00);
    base.timeout = false;
    base.delta = 200;

    base.init = function (){
      base.options = $.extend({},$.resizeend.defaultOptions, options);

      if (base.options.runOnStart) {
        base.options.onDragEnd();
      }

      $(base.el).resize(function () {
        base.rtime = new Date();
        if (base.timeout === false) {
          base.timeout = true;
          setTimeout(base.resizeend, base.delta);
        }
      });
    };

    base.resizeend = function () {
      if (new Date() - base.rtime < base.delta) {
        setTimeout(base.resizeend, base.delta);
      }
      else {
        base.timeout = false;
        base.options.onDragEnd();
      }
    };

    base.init();
  };

  /**
   * The default options for the resizeend function.
   */
  $.resizeend.defaultOptions = {
    onDragEnd: function () {},
    runOnStart: false
  };

  /**
   * Wrapper for the resizeend function.
   *
   * @param options
   *   The options for the resizeend event.
   */
  $.fn.resizeend = function (options){
    return this.each(function (){
      (new $.resizeend(this, options));
    });
  };

  /**
   * Check whether a media query currently applies.
   *
   * @param query
   *   The media query to check.
   */
  $.matchMedia = function(query) {
    var bool,
      docElem  = document.documentElement,
      refNode  = docElem.firstElementChild || docElem.firstChild,
      // fakeBody required for <FF4 when executed in <head>
      fakeBody = document.createElement('body'),
      div      = document.createElement('div');

    div.id = 'omega-mediaquery-test';
    div.style.cssText = "position:absolute;top:-100em";
    fakeBody.style.background = "none";
    fakeBody.appendChild(div);

    div.innerHTML = '&shy;<style media="'+query+'"> #omega-mediaquery-test { width: 42px; } </style>';

    docElem.insertBefore(fakeBody, refNode);
    bool = div.offsetWidth == 42;
    docElem.removeChild(fakeBody);

    return bool;
  };

  // Register a media query for testing purposes.
  $.extend(Drupal.omega.mediaQueries, {foo: 'all and (max-width: 500px)'});

  var mediaQueryStatus = {};

  /**
   * Attach the resizeend event to the window.
   */
  $(window).resizeend({
    onDragEnd: function () {
      $.each(Drupal.omega.mediaQueries, function (index, element) {
        var bool = $.matchMedia(element);

        if (!mediaQueryStatus.hasOwnProperty(index) || mediaQueryStatus[index] === undefined || bool != mediaQueryStatus[index]) {
          console.log(index);
          mediaQueryStatus[index] = bool;
        }
      });
    },
    runOnStart: true
  });

})(jQuery);