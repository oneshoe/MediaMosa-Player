// $Id: fideo.js 276 2011-12-05 09:07:33Z thijs $
var Drupal = Drupal || { 'settings': {}, 'behaviors': {}, 'themes': {}, 'locale': {} };
Drupal.settings.fideo = {};
Drupal.fideoCache = {};

(function ($) {

Drupal.behaviors.fideo = {

  attach: function(context) {
    // First pass, to find all elements and initialize.
    for (k in Drupal.settings.fideo) {
      if ($('#'+ k, context).get(0)) {
        var fideo = new Drupal.fideo(k, Drupal.settings.fideo[k]);
        fideo.init();
      }
    }

    // Second pass for setting up the player.
    for (var k in Drupal.fideoCache) {
      if (!Drupal.fideoCache[k].setupDone) {
        Drupal.fideoCache[k].setup();
      }
    }  
  }
  
}

Drupal.fideo = function(el, config) {
  this.config = {
    type: '',
    multi: false,
    timecodes: false,
    target: ''
  };
  this.setupDone = false;
  this.multiChildren = [];
  this.parent = null;
  this.el = el;
  this.listeners = {};
  this.currentSlideIndex = 0;

  // Store global.
  Drupal.fideoCache[el] = this;

  // Event states.
  this.initDone = false;
  this.setupDone = false;

  jQuery.extend(this.config, config);
}

Drupal.fideo.prototype.init = function() {
  if (this.initDone) {
    return;
  }

  if (this.config.multi && this.registerParent(this.config.multi)) {
    this.parent.addMultiChild(this);
    
    var c = this;

    this.addListener('onSetup', function() {
      c.player.volume(0);
      c.hideControls();

      c.parent.player.addVideoListener('play', function() {
        c.syncTimecode(c.parent.player.currentTime());
        c.player.play();

        setInterval(function() {
          c.synced = false;
          c.syncTimecode(c.parent.player.currentTime());
        }, 5000);
      });
      
      c.parent.player.addVideoListener('pause', function() {
        c.player.pause();
        c.player.currentTime(this.currentTime());
      });
      
      c.parent.player.addVideoListener('waiting', function() {
        c.player.pause();
        c.syncTimecode(c.parent.player.currentTime());
      });
      
      c.parent.player.addVideoListener('seeked', function() {
        // @todo Check here if the parent is playing before calling play().
        c.player.play();
        c.syncTimecode(c.parent.player.currentTime());
      });

      // Configure onclick listener
      c.enlargePlayer();

      // Setup fullscreen mode
      $(c.parent.player.fullscreenControl).toggle(function () {
        c.setupFullscreenMode(true);
      }, function() {
        c.setupFullscreenMode(false);
      });
    });
  }

  if (this.config.type == 'slides' && this.registerParent(this.config.target)) {
    // For now, we have to check for slide changes with parent.
    var a = this;
    var maxh = $('#' + this.el).parent().height(),
      ratio = 0.75, maxw = maxh / ratio;
    
    $('.fideo-slide:first', this.obj).addClass('fideo-active');
    this.parent.addListener('onSetup', function() {
      this.setSlideTiming(a.config.timecodes);
    });
    this.parent.addListener('onSlideChange', function(n) {
      $('.fideo-active', a.obj).removeClass('fideo-active');
      $('.fideo-slide:eq('+ this.currentSlideIndex +')', a.obj).addClass('fideo-active');
    });

    $('.fideo-slides').toggle(function () {
      $(this).css({'z-index' : '100'});
      $(this).animate({'height' : maxh, 'width' : maxw}, 300);
    }, function () {
      var c = this;

      $(c).animate({'height' : 267 * ratio, 'width' : '267'}, 300);
      // Wait a little, till the animation is finished
      setTimeout(function () {
        $(c).css({'z-index' : '0'});
      }, 300);
    });
  }

  if (this.config.type == 'markers' && this.registerParent(this.config.target)) {
    // Style the player, to make space for the markers.
    $('.fideo-wrapper').css({'height' : ($('.fideo-wrapper').height() + $('#' + this.el).height() + 2) + "px"});
    $('#fideo-cfideo_video_2').css({'bottom' : $('#' + this.el).height() + 2});

    // Setup markers.
    var timecodeMarkers = $("#" + this.el);
    if (timecodeMarkers.length > 0 && this.parent.config.type == 'video') {
      var c = this;

      this.parent.addListener('onSetup', function() {
        var dur = null;
        if (typeof c.config.duration != 'undefined') {
          dur = c.parent.config.duration;
        }
        else if (typeof this.player.duration() != 'undefined') {
          dur = c.parent.player.duration();
        }

        if (dur) {
          c.setTimecodeMarkers(dur, timecodeMarkers);
        }
        else {
          this.player.addVideoListener('loadedmetadata', function() {
            c.setTimecodeMarkers(this.duration(), timecodeMarkers);
          });
        }
      });
    }
  }
  
  if (this.config.type == 'video' && !this.config.multi) {
    // Use the primary video to resize the stage.
    this.resizeContainer();
  }

  this.initDone = true;
}

Drupal.fideo.prototype.callListeners = function(type, arg) {
  if (typeof(this.listeners[type]) != 'undefined') {
    var l = this.listeners[type].length;
    for (var i = 0; i < l; i++) {
      this.listeners[type][i].call(arg);
    }
  }
}

Drupal.fideo.prototype.syncTimecode = function(ptc) {
  var c = this.player.currentTime();
  if (this.synced) return;
  if (ptc - c > 0.4 || ptc - c < 0.4) {
    this.player.currentTime(ptc + 0.2);
    this.synced = true;
  }
}

Drupal.fideo.prototype.addListener = function(type, callback) {
  if (typeof(this.listeners[type]) == 'undefined') {
    this.listeners[type] = [];
  }
  this.listeners[type].push(callback.context(this));
}

Drupal.fideo.prototype.addMultiChild = function(el) {
  this.multiChildren.push(el);
}

Drupal.fideo.prototype.setup = function() {
  if (this.setupDone) {
    return;
  }

  if (this.config.type == 'video') {
    var conf = {
      controlsBelow: false,
      controlsHiding: true,
      defaultVolume: 1,
      flashVersion: 9,
      linksHiding: true,
      playOnClick: true
    };
    if (this.config.multi) conf.playOnClick = false;
    this.player = new VideoJS(this.el, conf);
    this.player.fideoEl = this.el;
  }

  this.setupDone = true;
  this.callListeners('onSetup');
}

Drupal.fideo.prototype.resizeContainer = function() {
  var el = $('#' + this.el), elp = el.parent();
  var position = elp.parent().position();
  var availw = elp.parent().parent().width();
  
  var w = availw - position.left;
  var h = Math.round(w * this.config.ratio);
  elp.add(el).css({
    width: w,
    height: h
  }).find('.vjs-poster').css({
    width: w,
    height: h
  })
  elp.parent().parent().height(h);
}

Drupal.fideo.prototype.registerParent = function(el) {
  if (typeof(Drupal.fideoCache[el]) != 'undefined') {
    this.parent = Drupal.fideoCache[el];
    this.parent.init();
    return true;
  }
  return false;
}

Drupal.fideo.prototype.hideControls = function() {
  this.player.each(this.player.controlBars, function(bar){
    bar.style.display = "none";
    bar.style.visibility = "hidden";
  });
  this.player.options.controlsHiding = false;
}

/**
 * Set a callback function to be called every instance of the timed moment.
 */
Drupal.fideo.prototype.setSlideTiming = function(timecodes) {
  this.timedCache = [];

  // Index the slide items and their timings.
  var l = timecodes.length;
  for (var i = 0; i < l; i++) {
    this.timedCache[i] = {start: timecodes[i], end: (i+1 < l ? timecodes[i + 1] : 0)};
  }
  this.currentSlideIndex = 0;

  this.player.onCurrentTimeUpdate(this.playerTimeUpdate);
}

Drupal.fideo.prototype.playerTimeUpdate = function(timecode) {
  var a = Drupal.fideoCache[this.fideoEl];
  if (!a.timedCache) {
    return;
  }
  if (timecode >= a.timedCache[a.currentSlideIndex].start && ((a.timedCache[a.currentSlideIndex].end && timecode < a.timedCache[a.currentSlideIndex].end) || !a.timedCache[a.currentSlideIndex].end)) {
    // Not to worry.
  }
  else {
    // Other slide!
    var n = a.selectSlideIndex(timecode);
    if (a.currentSlideIndex != n) {
      a.currentSlideIndex = n;
      a.callListeners('onSlideChange');
    }
  }
}

Drupal.fideo.prototype.selectSlideIndex = function(timecode) {
  var idx = 0;
  for (var i = 0; i < this.timedCache.length; i++) {
    if (timecode >= this.timedCache[i].start) {
      idx = i;
    }
  }
  return idx;
}

Drupal.fideo.prototype.setupFullscreenMode = function(show) {
  var windowDimensions = {
    'width' :  $(window).width(),
    'height' : $(window).height()
  };

  if (windowDimensions.width > windowDimensions.height) {
    // widescreen (height is leading)
    var oldPlayerHeight = $('.fideo-wrapper').height();
    var newPlayerHeight = windowDimensions.height;

    var oldPlayerWidth = $('#fideo').width();
    var newPlayerWidth = windowDimensions.width;
    var scaleRatio = oldPlayerWidth / newPlayerWidth;

    var primaryDimensions = {
      'width' : $('#fideo-primary').width() / scaleRatio,
      'height' : $('#fideo-primary').height() / scaleRatio
    };

    var secondaryDimensions = {
      'width' : $('#fideo_video_2').width() / scaleRatio,
      'height' : $('#fideo_video_2').height() / scaleRatio
    };

    $('#fideo').addClass('fullscreen').css({'width' : windowDimensions.width});
    $('.fideo-wrapper').css({'height' : windowDimensions.height});

    $('#fideo-cfideo-primary').css({'height' : primaryDimensions.height, 'width' : primaryDimensions.width});
    $(this.parent.player.video).css({'height' : primaryDimensions.height, 'width' : primaryDimensions.width});
    $('#fideo-cfideo-primary .video-js-box').css({'height' : primaryDimensions.height, 'width' : primaryDimensions.width});

    $('.fideo-slides').css({'height' : secondaryDimensions.height, 'width' : secondaryDimensions.width});

    $('#fideo-cfideo_video_2').css({'height' : secondaryDimensions.height, 'width' : secondaryDimensions.width});
    $('#fideo_video_2').css({'height' : secondaryDimensions.height, 'width' : secondaryDimensions.width});
  }


  // if (show) {
  //   var width = (screen.width * 0.40);
  //   var height = (width / secondaryDimensions.originalWidth) * secondaryDimensions.originalHeight;
  //
  //   // Set slider dimensions
  //   $(slidesWrapper).width(width);
  //   $(slidesWrapper).height(height);
  //
  //   // Set player dimensions
  //   secondaryPlayer.player().width(width);
  //   secondaryPlayer.player().width(height);
  //
  //   $('#fideo').addClass('fullscreen');
  //   $('.vjs-fullscreen').css({'left' : 'inherit', 'right' : '0'});
  // }
  // else {
  //   // Set slider dimensions
  //   $(slidesWrapper).width(secondaryDimensions.originalWidth);
  //   $(slidesWrapper).height(secondaryDimensions.originalHeight);
  //
  //   // Set player dimensions
  //   secondaryPlayer.player().width(secondaryDimensions.originalHeight);
  //   secondaryPlayer.player().width(secondaryDimensions.originalWidth);
  //
  //   $('#fideo').removeClass('fullscreen');
  // }
}

Drupal.fideo.prototype.enlargePlayer = function() {
  var c = this;

  $(this.player.video).toggle(function() {
    $(this.player.video).css({'z-index' : '100'});
    $(this.player.video).animate({'height' : '400', 'width' : '534', 'bottom' : '200'}, 300);
  }, function() {
    $(this.player.video).animate({'height' : '200', 'width' : '267', 'bottom' : '0'}, 300);
    // Wait al little, till the animation is finished.
    setTimeout(function () {
      $(c.player.video).css({'z-index' : '0'});
    }, 300);
  });
}

Drupal.fideo.prototype.setTimecodeMarkers = function (duration, timecodeMarkers) {
  var c = this.parent;

  $.each($(timecodeMarkers).find('.timecode-marker'), function(key, element) {
    var hash = $(element).get(0).hash;
    var regex = /^#start:(\d+(\.\d+)?)\:end:(\d+(\.\d+)?)$/;
    var timecodes = hash.match(regex);

    // Marker specifications.
    var markerLength = timecodes[3] - timecodes[1];
    var markerWidthPercentage = (markerLength < 2) ? 0.5 : (markerLength / duration) * 100;
    var markerPosition = (timecodes[1] / duration) * 100;

    // Click handling, forwarding the playhead to new timecode.
    $(element).click(function () {
      c.player.currentTime(timecodes[1]);
      c.player.play();
    });

    // Set CSS properties.
    $(element).css({left: markerPosition +'%', width: markerWidthPercentage +"%", display: 'block'});

    var tooltipMessage = $(element).html();
    $(element).html('');

    var tw = $(element).parent().width();
    var sw = 150;
    var pxPos = Math.round((markerPosition / 100) * tw);
    var txPos = tgPos = 'Middle';
    if (pxPos - sw/2 < 0) {
      txPos = 'Left';
      tgPos = 'Right';
    }
    else if (pxPos + sw/2 > tw) {
      txPos = 'Right';
      tgPos = 'Left';
    }

    if (tooltipMessage) {
      $(element).qtip({
        content: tooltipMessage,
        hide: {when: 'mouseout', fixed: true},
        show: {delay: 0},
        style: {
          name: 'light',
          width: sw,
          tip: {
            corner: 'bottom'+ txPos,
            size: {x: 7, y: 7}
          },
          border: {
            width: 2,
            radius: 2,
            color: 'rgba(204,204,204,0.9)'
          }
        },
        position: {corner: {target: 'top'+ tgPos,tooltip: 'bottom'+ txPos}}
      });
    }
  });
}

// In case we're not in a Drupal behaviors environment.
if (typeof(Drupal.attachBehaviors) == 'undefined') {
  $(document).ready(function() {
    Drupal.behaviors.fideo(document);
  });
}

})(jQuery);