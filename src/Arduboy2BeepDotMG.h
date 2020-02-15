/**
 * @file Arduboy2BeepDotMG.h
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#ifndef ARDUBOY2_BEEP_DOTMG_H
#define ARDUBOY2_BEEP_DOTMG_H

// For backwards compatibility
#define BeepPin1 BeepChan1
#define BeepPin2 BeepChan2

/** \brief
 * Play simple square wave tones using speaker channel 1.
 *
 * \note
 * Class `BeepChan2` provides identical functions for playing tones on speaker
 * channel 2. Both classes can be used in the same sketch to allow playing
 * two tones at the same time. To do this, the `begin()` and `timer()`
 * functions of both classes must be used.
 *
 * \details
 * This class can be used to play square wave tones on speaker channel 1.
 * The functions are designed to produce very small and efficient code.
 *
 * A tone can be set to play for a given duration, or continuously until
 * stopped or replaced by a new tone. The program continues to run while a
 * tone is playing. A small amount of code is required to time and stop a
 * tone after a given duration.
 *
 * Although there's no specific code to handle mute control, the
 * `Arduboy2Audio` class will work since it has code to mute sound by setting
 * the speaker pin to input mode and unmute by setting the pin as an output.
 * The `BeepChan1` class doesn't interfere with this operation.
 *
 * The duration of tones is timed by calling the `timer()` function continuously
 * at a fixed interval. The duration of a tone is given by specifying the number
 * of times `timer()` will be called before stopping the tone.
 *
 * For sketches that use `Arduboy2::nextFrame()`, or some other method to
 * generate frames at a fixed rate, `timer()` can be called once per frame.
 * Tone durations will then be given as the number of frames to play the tone
 * for. For example, with a rate of 60 frames per second a duration of 30
 * would be used to play a tone for half a second.
 *
 * The variable named `#duration` is the counter that times the duration of a
 * tone. A sketch can determine if a tone is currently playing by testing if
 * the `#duration` variable is non-zero (assuming it's a timed tone, not a
 * continuous tone).
 *
 * To keep the code small and efficient, the frequency of a tone is specified
 * by the actual count value to be loaded into to timer/counter peripheral.
 * The frequency will be determined by the count provided and the clock rate
 * of the timer/counter. In order to allow a tone's frequency to be specified
 * in hertz (cycles per second) the `freq()` helper function is provided,
 * which converts a given frequency to the required count value.
 *
 * All members of the class are static, so it's not necessary to create an
 * instance of the class in order to use it. However, creating an instance
 * doesn't produce any more code and it may make the source code smaller and
 * make it easier to switch to the `BeepChan2` class if it becomes necessary.
 *
 * The following is a basic example sketch, which will generate a tone when
 * a button is pressed.
 *
 * \code{.cpp}
 * #include <Arduboy2DotMG.h>
 * #include <Arduboy2BeepDotMG.h>
 *
 * Arduboy2 arduboy;
 * BeepChan1 beep; // class instance for speaker channel 1
 *
 * void setup() {
 *   arduboy.begin();
 *   arduboy.setFrameRate(50);
 *   beep.begin(); // set up the hardware for playing tones
 * }
 *
 * void loop() {
 *   if (!arduboy.nextFrame()) {
 *     return;
 *   }
 *
 *   beep.timer(); // handle tone duration
 *
 *   arduboy.pollButtons();
 *
 *   if (arduboy.justPressed(A_BUTTON)) {
 *     // play a 1000Hz tone for 100 frames (2 seconds at 50 FPS)
 *     // beep.freq(1000) is used to convert 1000Hz to the required count
 *     beep.tone(beep.freq(1000), 100);
 *   }
 * }
 * \endcode
 *
 * \see BeepChan2
 */
class BeepChan1
{
 public:

  /** \brief
   * The counter used by the `timer()` function to time the duration of a tone.
   *
   * \details
   * This variable is set by the `dur` parameter of the `tone()` function.
   * It is then decremented each time the `timer()` function is called, if its
   * value isn't 0. When `timer()` decrements it to 0, a tone that is playing
   * will be stopped.
   *
   * A sketch can determine if a tone is currently playing by testing if
   * this variable is non-zero (assuming it's a timed tone, not a continuous
   * tone).
   *
   * Example:
   * \code{.cpp}
   * beep.tone(beep.freq(1000), 15);
   * while (beep.duration != 0) { } // wait for the tone to stop playing
   * \endcode
   *
   * It can also be manipulated directly by the sketch, although this should
   * seldom be necessary.
   */
  static uint8_t duration;

  /** \brief
   * Set up the hardware.
   *
   * \details
   * Prepare the hardware for playing tones.
   * This function must be called (usually in `setup()`) before using any of
   * the other functions in this class.
   */
  static void begin();

  /** \brief
   * Play a tone continually, until replaced by a new tone or stopped.
   *
   * \param freq The desired tone frequency, from 23 to 1500000 Hz.
   *
   * \details
   * A tone is played indefinitely, until replaced by another tone or stopped
   * using `noTone()`.
   *
   * \see freq() timer() noTone()
   */
  static void tone(float freq);

  /** \brief
   * Play a tone for a given duration.
   *
   * \param freq The desired tone frequency, from 23 to 1500000 Hz.
   * \param dur The duration of the tone, used by `timer()`.
   *
   * \details
   * A tone is played for the specified duration, or until replaced by another
   * tone or stopped using `noTone()`.
   *
   * The duration value is the number of times the `timer()` function must be
   * called before the tone is stopped.
   *
   * \see freq() timer() noTone()
   */
  static void tone(float freq, uint8_t dur);

  /** \brief
   * Handle the duration that a tone plays for.
   *
   * \details
   * This function must be called at a constant interval, which would normally
   * be once per frame, in order to stop a tone after the desired tone duration
   * has elapsed.
   *
   * If the value of the `duration` variable is not 0, it will be decremented.
   * When the `duration` variable is decremented to 0, a playing tone will be
   * stopped.
   */
  static void timer();

  /** \brief
   * Stop a tone that is playing.
   *
   * \details
   * If a tone is playing it will be stopped. It's safe to call this function
   * even if a tone isn't currently playing.
   *
   * \see tone()
   */
  static void noTone();

  /** \brief
   * This function was originally intended to convert a frequency to desired count
   * value for input to `tone()`. This is no longer necessary, as `tone()` now
   * takes a frequency directly. It is maintained here for compatibility.
   *
   * \see tone()
   */
  static constexpr float freq(const float hz)
  {
    return hz;
  }
};


/** \brief
 * Play simple square wave tones using speaker channel 2.
 *
 * \details
 * This class contains the same functions as class `BeepChan1` except they use
 * speaker channel 2 instead of speaker channel 1.
 *
 * See the documentation for `BeepChan1` for more details.
 *
 * \see BeepChan1
 */
class BeepChan2
{
 public:

  /** \brief
   * The counter used by the `timer()` function to time the duration of a tone
   * played on speaker channel 2.
   *
   * \details
   * For details see `BeepChan1::duration`.
   */
  static uint8_t duration;

  /** \brief
   * Set up the hardware for playing tones using speaker channel 2.
   *
   * \details
   * For details see `BeepChan1::begin()`.
   */
  static void begin();

  /** \brief
   * Play a tone on speaker channel 2 continually, until replaced by a new tone
   * or stopped.
   *
   * \param freq The desired tone frequency, from 23 to 1500000 Hz.
   *
   * \details
   * For details see `BeepChan1::tone(float)`.
   */
  static void tone(float freq);

  /** \brief
   * Play a tone on speaker channel 2 for a given duration.
   *
   * \param freq The desired tone frequency, from 23 to 1500000 Hz.
   * \param dur The duration of the tone, used by `timer()`.
   *
   * \details
   * For details see `BeepChan1::tone(float, uint8_t)`.
   */
  static void tone(float freq, uint8_t dur);

  /** \brief
   * Handle the duration that a tone on speaker channel 2 plays for.
   *
   * \details
   * For details see `BeepChan1::timer()`.
   */
  static void timer();

  /** \brief
   * Stop a tone that is playing on speaker channel 2.
   *
   * \details
   * For details see `BeepChan1::noTone()`.
   */
  static void noTone();

  /** \brief
   * This function was originally intended to convert a frequency to desired count
   * value for input to `tone()`. This is no longer necessary, as `tone()` now
   * takes a frequency directly. It is maintained here for compatibility.
   *
   * \see tone()
   */
  static constexpr float freq(const float hz)
  {
    return hz;
  }
};

#endif
