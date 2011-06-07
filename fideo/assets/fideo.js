// $Id: fideo.js 109 2011-06-07 14:39:55Z thijs $
var Drupal = Drupal || { 'settings': {}, 'behaviors': {}, 'themes': {}, 'locale': {} };
Drupal.settings.fideo = {};
Drupal.fideoCache = {};

Drupal.behaviors.fideo = function(context) {
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

    this.addListener('onSetup', function() {
      this.player.volume(0);
      this.hideControls();
      var c = this;

      this.parent.player.addVideoListener('play', function() {
        c.syncTimecode(c.parent.player.currentTime());
        c.player.play();

        setInterval(function() {
          c.syncTimecode(c.parent.player.currentTime());
        }, 2000);

      });
      this.parent.player.addVideoListener('pause', function() {
        c.player.pause();
        c.player.currentTime(this.currentTime());
      });
      this.parent.player.addVideoListener('waiting', function() {
        c.player.pause();
        c.syncTimecode(c.parent.player.currentTime());
      });
      this.parent.player.addVideoListener('seeked', function() {
        c.syncTimecode(c.parent.player.currentTime());
      });

      // Configure onclick listener
      this.enlargePlayer();

      // Setup fullscreen mode
      $(this.parent.player.fullscreenControl).toggle(function () {
        c.setupFullscreenMode(true);
      }, function() {
        c.setupFullscreenMode(false);
      });
    });
  }

  if (this.config.type == 'slides' && this.registerParent(this.config.target)) {
    // For now, we have to check for slide changes with parent.
    var a = this;
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
      $(this).animate({'height' : '400', 'width' : '534'}, 300);
    }, function () {
      var c = this;

      $(c).animate({'height' : '200', 'width' : '267'}, 300);
      // Wait al little, till the animation is finished
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
        if (typeof this.config.duration != 'undefined') {
          dur = this.config.duration;
        }
        else if (typeof this.player.duration() != 'undefined') {
          dur = this.player.duration();
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
  if (ptc - c > 0.3 || ptc - c < 0.3) {
    this.player.currentTime(ptc + 0.3);
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
    var a = this;
  }

  this.setupDone = true;
  this.callListeners('onSetup');
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

Drupal.fideo.prototype.setupFullscreenMode = function (show) {
  // Elements

  windowDimensions = {
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
      'height' : $('#fideo-primary').height() / scaleRatio,
    };

    var secondaryDimensions = {
      'width' : $('#fideo_video_2').width() / scaleRatio,
      'height' : $('#fideo_video_2').height() / scaleRatio,
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

Drupal.fideo.prototype.enlargePlayer = function () {
  var c = this;

  $(this.player.video).toggle(function () {
    $(this.player.video).css({'z-index' : '100'});
    $(this.player.video).animate({'height' : '400', 'width' : '534', 'bottom' : '200'}, 300);
  }, function () {
    $(this.player.video).animate({'height' : '200', 'width' : '267', 'bottom' : '0'}, 300);
    // Wait al little, till the animation is finished
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
