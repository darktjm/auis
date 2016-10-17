#ifndef _PLAY_H_
#define _PLAY_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/* play.H
	play notes
*/

/** Play simple sounds.
 *  Originally, this class provided routines for playing sounds on an IBM
 *  PC/RT's keyboard.  It now uses a portable audio library to play sounds
 *  on any modern sound system (but not the IBM PC/RT's keyboard any more,
 *  or any plain beeper any more, for that matter).  The supported sounds
 *  are still pretty much the same, though:  only frequency and duration
 *  may be adjusted.
 */
class play : public ATK {
public:
    virtual ATKregistryEntry *ATKregistry();

    static void Volume(long v);
    /**< Set overall volume for playback.  0 is off, and
     *   9 is high.  Unlike the RT, all 9 volume levels are different,
     *   and no attempt is made to map the levels to the RT's behavior.
     *   \param v The volume level.  It is clamped 0 <= \p v <= 9.
     */
    static void Retard(long v);
    /**< Set overall retard for playback.
     *   Note durations are multiplied by (1+Retard/50),
     *   so negative values increase speed.
     *   \param v The new retard value.  It is clampbed to -30 <= \p v <= 40.
     */
    static void Tuning(const char *name);
    /**< Choose scale temper.
     *   Choices are:
     *       - Pythagorean tuning
     *       - Just Intonation (Ptolemy's intentse diatonic scale)
     *       - Mean Tone
     *       - Equal Temperament (default)
     *   \param name The scale name.  Only the first letter of name is
     *               tested, and usupported values are ignored.
     */
    static void Tone(double freq, long duration, long volume);
    /**< Play the given frequency for the given time at the given volume.
     *   \param freq The frequency, in Hz.  Reaonable frequencies range from
     *               roughly 80.0 to 4000.0.
     *   \param duration The duration, in miliseconds.  This will be altered
     *                   by the global retard value.
     *   \param Volume The volume, in the range 0 .. 9 (clamped to valid
     *          values).  This is altered by the global volume level.
     */
    static void Notes(const char *s);
    /**< Play the sequence of notes in a string.
     *   The string has codes as defined for Microsoft BASIC:
     *   - \c O<em>x</em>: Choose octave.  The octave starts at
     *     C and ends with B.  O3 starts with middle C.  \e x is
     *     clamped to its valid values: 0 .. 6.  The default is 4.
     *   - \c &lt;: Change to the next lower octave.  Clamped to 0.
     *   - \c &gt;: Change to the next higher octave.  Clamped to 6.
     *   - \c L<em>x</em>: Set length of following note(s).  \e x
     *     is clamped to its valid values: 1 .. 64.  The length of the
     *     note is the length of a whole note divided by \e x.  The default
     *     is 4.  Generally this will be a power of 2.
     *   - \c P<em>x</em>: Rest (pause).  The length of the pause is as
     *     defined for \b L<em>x</em>, above.
     *   - \c C, \c C#, \c D, \c D#, \c E, \c F, \c F#, \c G, \c G#,
     *     \c A, \c A#, \c B: Play the named note in the current octave,
     *     volume and duration.  The duration may be extended by 50% by
     *     appending a dot (\c .).  More dots extend the time by 50% more for
     *     each one (50% of the extended duration, not the original).
     *   - \c C+, \c C-, \c E+, \c F-, \c F+, \c G-, \c G+, \c A-,
     *     \c A+, \c B-: alternate names for \c C#, \c E#, \c F#, \c G# and \c A#.
     *   - \c T<em>x</em>:  Set tempo.  The parameter \e x is the number
     *     of quarter notes in a minute.  Its value is clamped to its
     *     valid values:  32 .. 255.  The default is 120.
     *   - \c N<em>x</em>: Play a note.  This plays the \e x'th note.
     *     Values are clamped to the valid range (0 .. 84).  0 indicates
     *     silence.  Notes 1-12 are in octave 0, 13-24 in octave 1, etc.
     *     Middle C is note 37.
     *   - \c V<em>x</em>: Set volume.  The parameter is clamped to its
     *     valid range (1 .. 9).   The default is 3.
     *   - \c M<em>c</em>: Unknown.  Ignored, along with its one-character
     *     parameter.
     *   - \c X<em>var</em>; Ignored.  The parameter is terminated by
     *     the first semicolon.
     *   - Any other characters between commands are ignored.  In particular,
     *     whitespace may be interspersed liberally.
     *   Any numeric parameter above whose form is \c =<em>var</em>;
     *   rather than an integer is replaced by the minimum valid value.
     *   The \e var operand is terminated by the first semicolon.
     *   \param s The musical piece to play.
     */
    static void SetBufLen(int len);
    /**< Set the length of the sound buffer.
     *   Rather than play each note request from Tone() or Notes() (or
     *   their ness equivalents), notes are added to a queue, which is
     *   processed asynchronously whenever it is non-empty.  Adding more
     *   notes will only block if the queue is full, and notes will
     *   continue to play after the ruotines exit.  The minimum length is
     *   one.  The default is 5, which seems to be what the PC/RT used.
     *   It is not possible at preset to tell when the queue is finished
     *   processing.  One way to ensure it is finished is to play as
     *   many short silences as there are queue entries.  It is safe to
     *   set a new queue length at any time, even if there are notes
     *   playing.  If the new length is shorter than the number of
     *   queued up 
     *   \param len The new queue length.
     */
};
#endif
