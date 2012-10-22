/*********************************************************************
 * Railuino - Hacking your Marklin
 *
 * (c) 2012 Joerg Pleumann
 * 
 * TODO: Add proper license text here.
 * 
 * This library allows you to control your digital Marklin railway
 * using Arduino. It can be used in two flavors:
 *
 * (1) As a Mobile Station 2. You can use a Sparkfun CAN shield and a
 *     cable to connect your Arduino to the Digital Connector Box of
 *     a Mobile Station 2 and probably (untested) also to a Central
 *     Station 2. This allows you to control locomotives, functions
 *     and turnouts using MM2, DCC and other protocols. You can even
 *     read and write decoder CVs, as long as the decoder's protocol
 *     supports it (DCC does both, MM2 only write, others vary).
 *
 * (2) As an infrared controller. You can use a simple infrared LED
 *     and a resistor to have your Arduino talk to the very basic IR
 *     receiver box that comes with various starter packages. This
 *     allows you to control four MM2 locomotives on fixed addresses
 *     (the classic Delta addresses). You can also control sixteen
 *     turnouts (this is an undocumented feature of the IR box, and
 *     the original Marklin IR controller doesn't support it).
 *
 * In addition to controlling things on and connected to the track
 * the library allows you to receive reports about track usage using
 * the standard S88 bus.
 */

#ifndef railuino__h
#define railuino__h

#include <Arduino.h>

// ===================================================================
// === Common definitions ============================================
// ===================================================================

/**
 * Constants for protocol base addresses.
 */
#define ADDR_MM2     0x0000 // MM2 locomotive
#define ADDR_SX1     0x0800 // Selectrix (old) locomotive
#define ADDR_MFX     0x4000 // MFX locomotive
#define ADDR_SX2     0x8000 // Selectrix (new) locomotive
#define ADDR_DCC     0xC000 // DCC locomotive
#define ADDR_ACC_SX1 0x2000 // Selectrix (old) magnetic accessory
#define ADDR_ACC_MM2 0x2FFF // MM2 magnetic accessory
#define ADDR_ACC_DCC 0x3800 // DCC magnetic accessory

/**
 * Constants for classic MM2 Delta addresses.
 */
#define DELTA1       78
#define DELTA2       72
#define DELTA3       60
#define DELTA4       24

/**
 * Constants for locomotive directions.
 */
#define DIR_CURRENT  0
#define DIR_FORWARD  1
#define DIR_REVERSE  2
#define DIR_CHANGE   3

/**
 * Constants for accessory states.
 */
#define ACC_OFF      0
#define ACC_ROUND    0
#define ACC_RED      0
#define ACC_RIGHT    0
#define ACC_HP0      0

#define ACC_ON       1
#define ACC_GREEN    1
#define ACC_STRAIGHT 1
#define ACC_HP1      1

#define ACC_YELLOW   2
#define ACC_LEFT     2
#define ACC_HP2      2

#define ACC_WHITE    3
#define ACC_SH0      3

/**
 * Represents a message going through the Marklin CAN bus. More or
 * less a beautified version of the real CAN message. You normally
 * don't need to use this unless you want to experiment with the
 * protocol or extend the library. See the Marklin protocol
 * documentation for details.
 */
typedef struct {

  /**
   * The command number.
   */
  byte command;
  
  /**
   * The hash that is used for avoiding device/message collisions.
   */
  word hash;

  /**
   * Whether this is a response to a request.
   */
  boolean response;

  /**
   * The number of data bytes in the payload.
   */
  byte length;

  /**
   * The actual message data bytes.
   */
  byte data[8];

} TrackMessage;

// ===================================================================
// === TrackController ===============================================
// ===================================================================

/**
 * Controls things on and connected to the track: locomotives,
 * turnouts and other accessories. While there are some low-level
 * methods for dealing with messages, you will normally want to use
 * the high-level methods that wrap most of the the nasty protocol
 * details. When addressing something, you have to tell the system
 * the type of address (or decoder) you are using by adding the
 * proper protocol base address. For instance, DCC locomotive 42
 * is properly addressed as ADDR_DCC + 42.
 */
class TrackController {

    private:
	
	bool mDebug;
	
	int mHash;
	
	bool mLoopback;

    public:

	/**
	 * Creates a new TrackController and does some initializing.
	 */
    TrackController(int hash, bool debug);

    TrackController(int hash, bool debug, bool loopback);

    /**
     * Sends a message and reports true on success. Internal method.
     * Normally you don't want to use this, but the more convenient
     * methods below instead.
     */
    boolean sendMessage(TrackMessage *message);

    /**
     * Receives an arbitrary message, if available, and reports true
     * on success. Does not block. Internal method. Normally you
     * don't want to use this, but the more convenient methods below
     * instead.
     */
    boolean receiveMessage(TrackMessage *message);

    /**
     * Sends a message and waits for the corresponding response,
     * returning true on success. Blocks until either a message with
     * the same command ID and the response marker arrives or the
     * timeout (in ms) expires. All non-matching messages are
     * skipped. Internal method. Normally you don't want to use this,
     * but the more convenient methods below instead.
     */
    boolean exchangeMessage(TrackMessage *message, word timeout);

    /**
     * Clears a message, setting all values to zero. Internal method.
     * Normally you don't want to use this, but the more convenient
     * methods below instead.
     */
    void clearMessage(TrackMessage *message);

    /**
     * Dumps the given message to the serial console if debuggin is
     * enabled. Internal method. Normally you don't want to use this,
     * but the more convenient methods below instead.
     */
    void printMessage(TrackMessage *message);

    void begin();

    /**
     * Controls power on the track. When passing false, all
     * locomotives will stop, but remember their previous directions
     * and speeds. When passing true, all locomotives will regain
     * their old directions and speeds. The system starts in
     * stopped mode in order to avoid accidents. The return value
     * reflects whether the call was successful.
     */
    boolean setPower(bool power);
    
    /**
     * Sets the direction of the given locomotive. Valid directions
     * are those specified by the DIR_* constants. The return value
     * reflects whether the call was successful.
     */
    boolean setLocoDirection(word address, byte direction);

    /**
     * Toggles the direction of the given locomotove. This normally
     * includes a full stop.
     */
    boolean toggleLocoDirection(word address);
    
    /**
     * Sets the speed of the given locomotive. Valid speeds are
     * 0 to 1023 (inclusive).  The return value reflects whether
     * the call was successful.
     */
    boolean setLocoSpeed(word address, word speed);

    /**
     * Increases the speed of the given locomotive by 1/14th
     * of the maximum speed.
     */
    boolean accelerateLoco(word address);

    /**
     * Decreases the speed of the given locomotive by 1/14th
     * of the maximum speed.
     */
    boolean decelerateLoco(word address);

    /**
     * Sets the given function of the given locomotive (or simply a
     * function decoder). Valid functions are 0 to 31, with 0
     * normally denoting the head/backlight. Valid values are, again,
     * 0 ("off") to 31, although not all protocols support values
     * beyond 1 (which then means "on").  The return value reflects
     * whether the call was successful.
     */
    boolean setLocoFunction(word address, byte function, byte power);
    
    /**
     * Toggles the given function of the given locomotive. Valid
     * functions are 0 to 31, with 0 normally denoting the
     * head/backlight.
     */
    boolean toggleLocoFunction(word address, byte function);
    
    /**
     * Switches the given magnetic accessory. Valid position values
     * are those denoted by the ACC_* constants. Valid power values
     * are 0 ("off") to 31 (inclusive) although not all protocols
     * support values beyond 1 (which then means "on"). The final
     * parameter specifies the time (in ms) for which the accessory
     * will be active. A time of 0 means the accessory will only be
     * switched on. Some magnetic accessories must not be active for
     * too long, because they might burn out. A good timeout for
     * Marklin turnouts seems to be 20 ms. The return value reflects
     * whether the call was successful.
     */
    boolean setAccessory(word address, byte position, byte power, word time);
    
    /**
     * Switches a turnout. This is actually a convenience function
     * around setAccessory() that uses default values for some
     * parameters. The return value reflects whether the call was
     * successful.
     */
    boolean setTurnout(word address, boolean straight);
    
    /**
     * Queries the direction of the given locomotive and writes it
     * into the referenced byte. The return value indicates whether
     * the call was successful and the direction is valid.
     */
    boolean getLocoDirection(word address, byte *direction);
    
    /**
     * Queries the speed of the given locomotive and writes it
     * into the referenced byte. The return value indicates whether
     * the call was successful and the speed is valid.
     */
    boolean getLocoSpeed(word address, word *speed);

    /**
     * Queries the given function of the given locomotive and writes
     * it into the referenced byte. The return value indicates
     * whether the call was successful and the power is valid. Note
     * that the call will not reflect the original power value sent
     * to the function, but only 0 ("off") or 1 ("on").
     */
    boolean getLocoFunction(word address, byte function, byte *power);
    
    /**
     * Queries the given magnetic accessory's state and and writes
     * it into the referenced bytes. The return value indicates
     * whether the call was successful and the bytes are valid. Note
     * that the call will not reflect the original power value sent
     * to the function, but only 0 ("off") or 1 ("on").
     */
    boolean getAccessory(word address, byte *position, byte *power);

    /**
     * Queries a turnout state. This is actually a convenience
     * function around getAccessory() that uses default values for
     * some parameters.
     */
    boolean getTurnout(word address, boolean *straight);

    /**
     * Writes the given value to the given config number of the given
     * locomotive.  The return value reflects whether the call was
     * successful.
     */
    boolean writeConfig(word address, word number, byte value);
    
    /**
     * Reads the given config number of the given locomotive into the
     * given value.
     */
    boolean readConfig(word address, word number, byte *value);

};

/**
 * The little brother of TrackController. This class talks to an IR
 * connector box. The interface is more or less a subset of the one
 * that TrackController offers. There are exactly four locomotives,
 * numbered 1 to 4, and sixteen turnouts, numbered 1 to 16. The IR
 * controller does not care about protocols, although in reality
 * it's all MM2. Also, no object-oriented mumbo-jumbo here to
 * accomodate for the limited Arduino resources. Note this class
 * is stateless, so the caller has to keep track of things, if
 * necessary.
 */
class TrackControllerInfrared {

    private:
	
	bool mDebug;
	
	word mToggle;
	
	boolean mPower;
	
	/**
	 * Information about our four locomotives. One word per
	 * loco. The bits are:
	 * 
	 * 0..3 speed     (0 to 14)
	 *    4 direction (1 = backward)
	 *    5 light     (1 = on, actually function zero)
	 * 6..9 functions (1 = on)
	 */
	word mLocoInfo[4];
	
	/**
	 * Information about the turnouts, in order of bits.
	 * (1 = straight).
	 */
	word mTurnoutInfo;
	
    public:

	/**
	 * Creates a new TrackControllerInfrared and does some
	 * initializing.
	 */
    TrackControllerInfrared(bool debug);

    /**
     * Sends a message consisting of address and command (in the
     * sense of RC5). Takes care of the extension and alternating
     * bits. Internal method. Normally you don't want to use this,
     * but the more convenient methods below instead.
     */
    boolean sendMessage(word address, word command);

    /**
     * Controls power on the track. When passing false, all
     * locomotives will stop, but remember their previous directions
     * and speeds. When passing true, all locomotives will regain
     * their old directions and speeds. The system starts in
     * stopped mode in order to avoid accidents. The return value
     * reflects whether the call was successful.
     */
    boolean setPower(boolean power);
    
    /**
     * Toogles the direction of the given locomotive. This includes
     * a full stop of the locomotive. The return value reflects whether
     * the call was successful.
     */
    boolean toggleLocoDirection(int loco);
    
    /**
     * Accelerates the given locomotive by 1 level on a scale assuming
     * a total of 14 speed levels. Does not have any effect if the
     * maximum speed has already been reached.
     */
    boolean accelerateLoco(int loco);

    /**
     * Accelerates the given locomotive by 1 level on a scale assuming
     * a total of 14 speed levels. Does not have any effect if the
     * maximum speed has already been reached.
     */
    boolean decelerateLoco(int loco);
    
    /**
     * Toggles the given function of the given locomotive (or simply a
     * function decoder). Valid functions are 0 to 4, with 0 normally
     * denoting the head/backlight. The return value reflects whether
     * the call was successful.
     */
    boolean toggleLocoFunction(int loco, int function);
    
    /**
     * Switches a turnout. The return value reflects whether the call
     * was successful.
     */
    boolean setTurnout(int turnout, boolean straight);

};

// ===================================================================
// === TrackReporter =================================================
// ===================================================================

/**
 * Implements the S88 bus protocol for reporting the state of tracks.
 * S88 is basically a long shift register where each bit corresponds
 * to a single contact on the track. Flip-flops on each S88 board make
 * sure activations are stored, so it is not necessary to query a
 * contact at the exact time it is activated. This implementation
 * allows a maximum of 512 bits or 32 full-width (16 bit) S88 boards.
 * The S88 standard recommends a maximum of 30 boards, so we should be
 * on the safe side.
 */
class TrackReporter {

  private:

  /**
   * The number of contacts available.
   */
  int mSize;

  /**
   * The most recent contact values we know.
   */ 
  byte mSwitches[64];

  public:

  /**
   * Creates a new TrackReporter with the given number of contacts.
   * While this can be safely set to the maximum of 512, it makes
   * sense to specify the actual number, since this speeds up
   * reporting.
   */
  TrackReporter(int size);
  
  /**
   * Reads the current state of all contacts into the TrackReporter
   * and clears the flip-flops on all S88 boards. Call this method
   * periodically to have up-to-date values.
   */
  void refresh();

  /**
   * Returns the state of an individual contact. Valid index values
   * are 1 to 512.
   */
  boolean getValue(int index);

};

#endif
