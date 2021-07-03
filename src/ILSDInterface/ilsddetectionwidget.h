/*  Copyright 2021 Philippe Even, Phuc Ngo and Pierre Even,
      co-authors of paper:
      Even, P., Grzesznik, A., Gebhardt, A., Chenal, T., Even, P. and Ngo, P.,
      2021,
      Fast extraction of linear structures fromLiDAR raw data
      for archaeomorphological structure prospection.
      In the International Archives of the Photogrammetry, Remote Sensing
      and Spatial Information Sciences (proceedings of the 2021 edition
      of the XXIVth ISPRS Congress).

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "pt2i.h"
#include "ipttileset.h"
#include "ctrackdetector.h"
#include "ridgedetector.h"
#include "ilsdcrossprofileview.h"
#include "ilsdlongprofileview.h"
#include "terrainmap.h"
#include "asImage.h"

class ASPainter;
class IniLoader;


/**
 * @class ILSDDetectionWidget ilsddetectionwidget.h
 * \brief Main widget of the interactive linear structure detection.
 *   Handles detection control and display.
 */
class ILSDDetectionWidget
{
public:

  /** Version number */
  static const std::string VERSION;

  /** Detector mode : simple analysis. */
  static const int MODE_NONE;
  /** Detector mode : detection of carriage tracks. */
  static const int MODE_CTRACK;
  /** Detector mode : detection of ridges. */
  static const int MODE_RIDGE;
  /** Detector mode : detection of hollow structures. */
  static const int MODE_HOLLOW;
  /** Detector mode : detection of ridge or hollow structures. */
  static const int MODE_RIDGE_OR_HOLLOW;


  /**
   * \brief Creates an interactive linear structure detection widget.
   */
  ILSDDetectionWidget ();

  /**
   * \brief Deletes the ILSD widget.
   */
  virtual ~ILSDDetectionWidget ();

  /**
   * \brief Closes analysis views, resets the detectors and unselects.
   */
  void reset ();

  /**
   * \brief Saves application settings.
   * @param iload Setting handler.
   */
  void saveSettings (IniLoader *iload);

  /**
   * \brief Saves application settings.
   * @param path Settings file name.
   */
  void saveSettings (const std::string& path);

  /**
   * \brief Restores application settings.
   * @param iload Setting handler.
   */
  void loadSettings (IniLoader *iload);

  /**
   * \brief Restores application settings.
   * @param path Settings file name.
   */
  void loadSettings (const std::vector<std::string>& path);

  /**
   * \brief Saves current detected structure.
   * @param path Structure file name.
   */
  void saveStructure (const std::string& path);

  /**
   * \brief Loads a detected structure.
   * @param path Structure file name.
   */
  void loadStructure (const std::vector<std::string>& path);

  /**
   * \brief Exports selected structure in SHP format.
   * @param path Shape file name.
   */
  void exportShape (const std::string& path);

  /**
   * \brief Saves current measure on ridge profile.
   * @param path Structure file name.
   */
  void saveMeasure (const std::string& path);

  /**
   * \brief Loads a measure on a ridge profile.
   * @param path Structure file name.
   */
  void loadMeasure (const std::vector<std::string>& path);

  /**
   * \brief Inverts the input stroke and runs a new detection.
   */
  void invertInputStroke ();

  /**
   * \brief Returns default stroke file name. 
   */
  const std::string getStrokeFileName () const;

  /**
   * \brief Saves current input stroke. 
   * @param Stroke file name.
   */
  void saveStroke (const std::string& path);

  /**
   * \brief Loads and runs an input stroke test.
   * @param Stroke file name.
   */
  void loadStroke (const std::vector<std::string>& path);

  /**
   * \brief Adds detected structure to a selection.
   * @param Selection file name.
   */
  void addToSelection (const std::string& path);

  /**
   * \brief Loads a selection of detected structures.
   * @param path Selection file name.
   */
  void loadSelection (const std::vector<std::string>& path);

  /**
   * \brief Saves screen shot.
   * @param path Capture file name.
   */
  void saveScreen (const std::string& path);

  /**
   * \brief Checks whether tiles are loaded.
   */
  inline bool tilesLoaded () const { return tiles_loaded; }

  /**
   * \brief Selects tiles to display.
   * @param paths Tile names.
   */
  void selectTiles (const std::vector<std::string>& paths);

  /**
   * \brief Loads tiles from standard tile file.
   */
  void loadTiles ();

  /**
   * \brief Creates and distributes DTM images.
   */
  void createMap ();

  /**
   * \brief Returns widget size.
   */
  ASCanvasPos widgetSize ();

  /**
   * \brief Builds and returns the image bitmap.
   */
  int** getBitmap (const ASImage& image);

  /**
   * \brief Returns the grid subdivision factor.
   */
  inline int gridSubdivisionFactor () const { return SUBDIV; }

  /**
   * \brief Returns the point cloud access type.
   */
  inline int cloudAccess () const { return cloud_access; }

  /**
   * \brief Sets the point cloud access type.
   * @param type New access type.
   */
  void setCloudAccess (int type);

  /**
   * \brief Returns the detection mode.
   */
  inline int mode () const { return det_mode; }

  /**
   * \brief Returns the type of background of the widget.
   */
  inline int getBackground () const { return background; }

  /**
   * \brief Sets the type of background of the widget.
   * @param bg New type of background.
   */
  inline void setBackground (int bg) { background = bg; }

  /**
   * \brief Displays the detected carriage track.
   */
  void display ();

  /**
   * \brief Returns whether tile bounds are displayed.
   */
  inline bool isDisplayTileOn () const { return tiledisp; }

  /**
   * \brief Switches the tile bound display modality.
   */
  inline void switchDisplayTile () { tiledisp = ! tiledisp; }

  /**
   * \brief Returns the display style used for selections.
   */
  inline int getSelectionStyle () const { return sel_style; }

  /**
   * \brief Sets the display style of selections.
   * @param style New selection display style.
   */
  inline void setSelectionStyle (int style) { sel_style = style; }

  /**
   * \brief Toggles the display style of selections.
   */
  void toggleSelectionStyle ();

  /**
   * \brief Returns the display style used for ridges.
   */
  inline int getRidgeStyle () const { return ridge_style; }

  /**
   * \brief Sets the display style of ridges.
   * @param style New ridge display style.
   */
  inline void setRidgeStyle (int style) { ridge_style = style; }

  /**
   * \brief Returns the display style used for carriage tracks.
   */
  inline int getTrackStyle () const { return ctrack_style; }

  /**
   * \brief Sets the display style of carriage tracks.
   * @param style New carriage track display style.
   */
  inline void setTrackStyle (int style) { ctrack_style = style; }

  /**
   * \brief Toggles the display style of detected structures.
   */
  void toggleStructureStyle ();

  /**
   * \brief Returns whether detection results are displayed.
   */
  inline bool isDisplayDetectionOn () const { return disp_detection; }

  /**
   * \brief Switches the detection result display modality.
   */
  inline void switchDisplayDetection () { disp_detection = ! disp_detection; }

  /**
   * \brief Returns whether road plateau display are smoothed.
   */
  inline bool isPlateauSmoothedOn () const { return smoothed_plateaux; }

  /**
   * \brief Switches the road plateau smoothed display modality.
   */
  inline void switchPlateauSmoothed () {
    smoothed_plateaux = ! smoothed_plateaux; }

  /**
   * \brief Returns whether ridge bump display are smoothed.
   */
  inline bool isBumpSmoothedOn () const { return smoothed_bumps; }

  /**
   * \brief Switches the ridge bump smoothed display modality.
   */
  inline void switchBumpSmoothed () { smoothed_bumps = ! smoothed_bumps; }

  /**
   * \brief Gets the status of selected structures display modality.
   */
  inline bool getSelectionDisplay () const { return disp_saved; }

  /**
   * \brief Sets the selected structures display modality.
   */
  inline void setSelectionDisplay (bool status) { disp_saved = status; }

  /**
   * \brief Updates the Qt widget display.
  void paint (QPainter *painter,
              const QStyleOptionGraphicsItem *option, QWidget *widget);
   */

  /**
   * \brief Returns the reference to the DTM map.
   */
  inline TerrainMap *getDtmMap () { return &dtm_map; }

  /**
   * \brief Returns the reference to the analysis controller.
   */
  inline ILSDItemControl *getAnalysisController () { return &ictrl; }

  /**
   * \brief Returns the reference to the carriage track detector.
   */
  inline CTrackDetector *getCTrackDetector () { return &tdetector; }

  /**
   * \brief Returns the reference to the ridge or hollow structure detector.
   */
  inline RidgeDetector *getRidgeDetector () { return &rdetector; }

  /**
   * \brief Rebuilds the background image after lighting modification.
   */
  void rebuildImage ();

  /**
   * \brief Checks whether window title should change.
   */
  bool titleChanged ();

  /**
   * \brief Checks whether window title should change.
   */
  inline void setTitleChanged (bool status) { new_title = status; }

   /**
    * \brief Returns a title for the window.
    */
  string getTitle () const;

   /**
    * \brief Toggles the background image.
    */
  void toggleBackground ();

  /**
   * \brief Returns the background intensity value.
   */
  inline int getBlackLevel () const { return (blevel); }

  /**
   * \brief Increments the background intensity value.
   * @param val Increment value.
   */
  void incBlackLevel (int val);

  /**
   * \brief Sets the background background intensity value.
   * @param val New background intensity value.
   */
  void setBlackLevel (int val);

  /**
   * \brief Returns the widget width.
   */
  inline int getWidth () const { return (width); }

  /**
   * \brief Returns the widget height.
   */
  inline int getHeight () const { return (height); }

  /**
   * \brief Returns the mouse position in widget coordinates.
   */
  inline bool getMousePosition (GLWindow *parent, ASCanvasPos &pos) const {
    return (augmentedImage.mouseToTexture (parent, pos)); }

  /**
   * \brief Captures the widget screen.
   * @param fname Capture file name.
   */
  void capture (string fname);

  /**
   * \brief Returns the widget X-shift value.
   */
  inline int getXShift () const { return (xShift); }

  /**
   * \brief Increments the widget X-shift value.
   * @param dir Increment direction.
   */
  inline void incXShift (int dir) { xShift += dir * MOVE_SHIFT; }

  /**
   * \brief Sets the widget X-shift value.
   * @param val New shift value.
   */
  inline void setXShift (int val) { xShift = val; }

  /**
   * \brief Returns the widget Y-shift value.
   */
  inline int getYShift () const { return (yShift); }

  /**
   * \brief Increments the widget Y-shift value.
   * @param dir Increment direction.
   */
  inline void incYShift (int dir) { yShift += dir * MOVE_SHIFT; }

  /**
   * \brief Sets the widget Y-shift value.
   * @param val New shift value.
   */
  inline void setYShift (int val) { yShift = val; };

  /**
   * \brief Returns the widget zoom value.
   */
  inline int getZoom () const { return (zoom); }

  /**
   * \brief Increments the widget zoom level.
   * @param dir Increment direction.
   */
  void incZoom (int dir);

  /**
   * \brief Sets the widget zoom value.
   * @param val New zoom value.
   */
  inline void setZoom (int val) { zoom = val; }

  /**
   * \brief Gets input stroke position.
   * @param p1 Input stroke start position to get.
   * @param p2 Input stroke end position to get.
   */
  inline void getInputStroke (Pt2i &pos1, Pt2i &pos2) {
    pos1.set (p1); pos2.set (p2); }

  /**
   * \brief Checks whether an input stroke has been defined.
   */
  inline bool isInputStrokeDefined () const { return udef; }

  /**
   * \brief Checks whether a valid input stroke has been defined.
   */
  inline bool isInputStrokeValid () const {
    return udef && ! p1.equals (p2); }

  /**
   * \brief Runs a detection using current input stroke.
   */
  inline void detect () { detect (p1, p2); }

  /**
   * \brief Runs a detection and displays the result.
   */
  void detectAndDisplay ();

  /**
   * \brief Saves the augmented image with extraction results.
   */
  bool saveAugmentedImage (const string& fileName, const char* fileFormat);

  /**
   * \brief Inquires if a cross profile view is opened.
   */
  inline bool isCrossProfileVisible () const { return (cp_view != NULL); }

  /**
   * \brief Inquires if a longitudinal profile view is opened.
   */
  inline bool isLongProfileVisible () const { return (lp_view != NULL); }

  /**
   * \brief Returns the opened cross profile view.
   */
  ILSDCrossProfileView *getCrossProfileView () const;

  /**
   * \brief Returns the opened longitudinal profile view.
   */
  ILSDLongProfileView *getLongProfileView () const;

  /**
   * \brief Toggles between available detection modes.
   */
  void toggleDetectionMode ();

  /**
   * \brief Sets the detection mode.
   * @param mode New detection mode.
   */
  void setDetectionMode (int mode);

  /**
   * \brief Switches cross profile analysis window on or off.
   */
  void switchCrossProfileAnalyzer ();

  /**
   * \brief Switches longitudinal profile analysis window on or off.
   */
  void switchLongProfileAnalyzer ();

  /**
   * \brief Inquires previous cross profile window position.
   *   Returns whether this position was fixed.
   * @param pos Position to update with previous value.
   */
  bool previousCrossProfilePosition (ASCanvasPos &pos) const;

  /**
   * \brief Inquires previous longitudinal profile window position.
   *   Returns whether this position was fixed.
   * @param pos Position to update with previous value.
   */
  bool previousLongProfilePosition (ASCanvasPos &pos) const;

  /**
   * \brief Sets ridge detector geometrical measure modality status.
   */
  void updateMeasuring ();

  /**
   * \brief Saves the last defined measure in default file.
   */
  void saveLastMeasure () const;

  /**
   * \brief Runs a performance test.
   * Displays the time spent for 1000 detections under the present stroke.
   */
  void performanceTest ();

  /**
   * \brief Enables key control.
   */
  void enableKeys ();

  /**
   * \brief Disables key control.
   */
  void disableKeys ();

  /**
   * \brief Checks whether keys are disabled or not.
   */
  inline bool keyDisabled () { return (popup_nb != 0); };

  /**
   * \brief Called when file selection is aborted or refused.
   */
  inline void noAction () { };


public:
  /**
   * \brief Clears the widget drawing.
   */
  void clearImage ();

  /**
   * @brief Request closure of the cross profile anaysis view.
   */
  void closeCrossProfileView ();

  /**
   * @brief Request closure of the longitudinal profile anaysis view.
   */
  void closeLongProfileView ();


public:

  /**
   * \brief Processes mouse press events.
   */
  void mousePressEvent (GLWindow* parentWindow);

  /**
   * \brief Processes mouse release events.
   */
  void mouseReleaseEvent (GLWindow* parentWindow);

  /**
   * \brief Processes move events.
   */
  void mouseMoveEvent (GLWindow* parentWindow);

  /**
   * \brief Requests a main window update.
   */
  void updateWidget ();

  /**
   * \brief Updates the widget drawing.
   */
  void paintEvent (GLWindow* drawWindow);

  /**
   *  \brief Resets the graphics view reference after any closing.
   */
  void itemDeleted (AsImGuiWindow *item);

  /**
   *  \brief Creates a new tile from a portion of the loaded tile (static).
   */
  void saveSubTile ();


private:

  /** Bump display mode : scans. */
  static const int DISP_SCANS;
  /** Bump display mode : connected scans. */
  static const int DISP_CONNECT;
  /** Bump display mode : scan bounds. */
  static const int DISP_BOUNDS;
  /** Bump display mode : bump top. */
  static const int DISP_SPINE;
  /** Bump display mode : bump center of mass. */
  static const int DISP_CENTER;
  /** Selection display mode : selection not displayed. */
  static const int SEL_NO;
  /** Selection display mode : thin selection stroke. */
  static const int SEL_THIN;
  /** Selection display mode : thick selection stroke. */
  static const int SEL_THICK;

  /** Background status : uniform black. */
  static const int BACK_BLACK;
  /** Background status : uniform white. */
  static const int BACK_WHITE;
  /** Background status : intensity image displayed. */
  static const int BACK_IMAGE;
  /** Background status : gradient image displayed. */
  static const int BACK_GRAD;
  /** Background status : X-gradient image displayed. */
  static const int BACK_GRADX;
  /** Background status : Y-gradient image displayed. */
  static const int BACK_GRADY;

  /** Default value for pen width. */
  static const int THIN_PEN;
  /** Default value for thick lines width. */
  static const int THICK_PEN;
  /** Default value for small track display width. */
  static const int SMALL_TRACK_WIDTH;
  /** Default value for large track display width. */
  static const int LARGE_TRACK_WIDTH;
  /** Default value for mean track display width. */
  static const int DEFAULT_MEAN_TRACK_WIDTH;
  /** Tolerence for segment picking (in count of naive lines) */
  static const int SELECT_TOL;

  /** Point cloud / Dtm image ratio. */
  static const int SUBDIV;
  /** DTM map move increment. */
  static const int MOVE_SHIFT;


  /** Initial scan start point. */
  Pt2i p1;
  /** Initial scan end point. */
  Pt2i p2;
  /** Saved scan start point. */
  Pt2i oldp1;
  /** Saved scan end point. */
  Pt2i oldp2;
  /** Flag indicating if the mouse is not dragging. */
  bool nodrag;
  /** Flag indicating if the mouse left button has been pressed. */
  bool bMousePressed;
  /** Flag indicating if picking mode is set. */
  bool picking;
  /** Flag indicating if a main window repaint is required. */
  bool to_update;
  /** Flag indicating if no external repaint is required. */
  bool with_aux_update;
  /** Flag indicating if the detection is user defined. */
  bool udef;
  /** Saved user definition flag. */
  bool oldudef;
  /** Key action inhibition status. */
  int popup_nb;

  int background;
  /** Black level used to lighten background images. */
  int blevel;
  /** Plateau insertion modality: accepted if true, status ok otherwise. */
  bool smoothed_plateaux;
  /** Bump insertion modality: accepted if true, status ok otherwise. */
  bool smoothed_bumps;
  /** Flag indicating whether detection stats should be output. */
  bool statsOn;
  /** Flag indicating whether detection result should be output. */
  bool verbose;
  /** Detected structure color. */
  ASColor structure_color;
  /** Selection stroke color. */
  ASColor selection_color;
  /** Analyzed scan color. */
  ASColor analyzed_color;
  /** Tile bound color. */
  ASColor tiles_color;
  /** Tile borders display modality. */
  bool tiledisp;

  /** Presently loaded image. */
  ASImage loadedImage;
  /** Present image augmented with processed data. */
  ASImage augmentedImage;
  /** Points cloud. */
  IPtTileSet ptset;
  /** Width of the present image. */
  int width;
  /** Height of the present image. */
  int height;
  /** Image to meter ratio : inverse of cell size. */
  float iratio;
  /** DTM cell size. */
  float cellsize;
  /** Cloud access speed level. */
  int cloud_access;

  /** Maximal window width. */
  int maxWidth;
  /** Maximal window height. */
  int maxHeight;
  /** Maximal value of x-scroll shift. */
  int xMaxShift;
  /** Maximal value of y-scroll shift. */
  int yMaxShift;
  /** X-scroll shift. */
  int xShift;
  /** Y-scroll shift. */
  int yShift;
  /** Window zoom. */
  int zoom;

  /** DTM normal map. */
  TerrainMap dtm_map;
  /** Detector mode. */
  int det_mode;
  /** Flag indicating if the window title should change. */
  bool new_title;
  /** Carriage track detector. */
  CTrackDetector tdetector;
  /** Ridge structure detector. */
  RidgeDetector rdetector;
  /** Cross profile view. */
  ILSDCrossProfileView* cp_view;
  /** Longitudinal profile view. */
  ILSDLongProfileView* lp_view;
  /** Analysis widget controls. */
  ILSDItemControl ictrl;

  /** Saved tracks map. */
  vector<Pt2i> savmap;
  /** Saved strokes. */
  vector<Pt2i> savstroke;
  /** Detected tracks display modality. */
  bool disp_detection;
  /** Saved structure display modality. */
  bool disp_saved;
  /** Carriage track display style. */
  int ctrack_style;
  /** Ridge or hollow display style. */
  int ridge_style;
  /** Selection display style. */
  int sel_style;
  /** Delineated track cuts display modality. */
  bool cuts_displayed;
  /** Delineated tracks width. */
  int track_width;
  /** Performance evaluation mode status. */
  bool perf_mode;

  /** Parameter registration server. */
  IniLoader *ini_load;
  /** Tiles loading status. */
  bool tiles_loaded;


  /**
   * \brief Saves current ridge.
   * @param ild File manager used.
   */
  void saveRidge (IniLoader *ild);

  /**
   * \brief Loads a detected ridge.
   * @param ild File manager used.
   */
  void loadRidge (IniLoader *ild);

  /**
   * \brief Saves current carriage track.
   * @param ild File manager used.
   */
  void saveCarTrack (IniLoader *ild);

  /**
   * \brief Loads a detected carriage track.
   * @param ild File manager used.
   */
  void loadCarTrack (IniLoader *ild);

  /**
   * \brief Saves current stroke.
   * @param ild File manager used.
   */
  void saveStroke (IniLoader *ild);

  /**
   * \brief Checks whether the stroke lies on the loaded DTM map.
   * @param ild File manager used.
   */
  bool checkStroke (IniLoader *ild);

  /**
   * \brief Loads and runs a stroke.
   * Returns whether the stroke is on the DTM map.
   * @param ild File manager used.
   */
  bool loadStroke (IniLoader *ild);

  /**
   * \brief Loads tiles.
   * @param path Tile file name.
   */
  void loadTiles (const std::string& path);

  /**
   * \brief Draws a list of points with the given color.
   * @param painter Drawing device.
   * @param pts List of points to be drawn.
   * @param color Drawing color.
   */
  void drawPoints (ASPainter& painter, vector<Pt2i> pts, ASColor color);

  /**
   * \brief Draws a list of image pixels.
   * @param painter Drawing device.
   * @param pix List of pixels to be drawn.
   */
  void drawPixels (ASPainter& painter, vector<Pt2i> pix);

  /**
   * \brief Draws the line joining two points.
   * @param painter Drawing device.
   * @param from Line start position.
   * @param to Line reach position.
   * @param color Drawing color.
   * @param width Drawing width.
   */
  void drawLine (ASPainter& painter, const Pt2i from, const Pt2i to,
                 ASColor color, int width = THIN_PEN);

  /**
   * \brief Draws a user selection.
   * @param painter Drawing device.
   * @param from Selection line start position.
   * @param to Selection line reach position.
   */
  void drawSelection (ASPainter& painter, const Pt2i from, const Pt2i to);

  /**
   * \brief Draws tile borders.
   * @param painter Drawing device.
   */
  void drawTiles (ASPainter& painter);

  /**
   * \brief Lighten the image according to the black level set.
   * @param im Image to lighten.
   */
  void lighten (ASImage& im);

  /**
   * \brief Writes the stats of the last detection in a file.
   */
  void writeStats ();

  /**
   * \brief Writes the result of the last detection in a file.
   */
  void writeDetectionResult ();

  /**
   * \brief Displays the saved blurred segments.
   */
  void displaySavedSegments ();

  /**
   * \brief Registers the last extracted blurred segment.
   * Returns the count of registered segments more.
   */
  int saveExtractedSegment ();

  /**
   * \brief Clears off the saved blurred segments.
   */
  void clearSavedSegments ();

  /**
   * \brief Outputs the last detection result status.
   */
  void writeDetectionStatus ();

  /**
   * \brief Detects a structure under input stroke.
   * @param p1 Input stroke start position.
   * @param p2 Input stroke end position.
   */
  void detect (const Pt2i& p1, const Pt2i& p2);

  /**
   * \brief Displays the window background (no detection).
   */
  void displayBackground ();

  /**
   * \brief Displays the result of the last detection.
   */
  void displayDetectionResult ();

  /**
   * \brief Displays straight strip bounds.
   * @param painter Display support.
   * @param from Straight strip start point.
   * @param to Straight strip end point.
   */
  void displayStraightStrip (ASPainter& painter,
                             const Pt2i from, const Pt2i to);

  /**
   * \brief Displays the analyzed scan in profile view.
   * @param painter Display support.
   */
  void displayAnalyzedScan (ASPainter& painter);

  /**
   * \brief Displays plateaux of detected carriage track.
   * @param painter Display support.
   * @param col Displayed track color.
   */
  void displayCarriageTrack (ASPainter& painter, ASColor col = ASColor::GREEN);

  /**
   * \brief Displays connected plateaux of detected carriage track.
   * @param painter Display support.
   * @param col Detected track display color.
   */
  void displayConnectedTrack (ASPainter& painter, ASColor col = ASColor::GREEN);

  /**
   * \brief Displays the detected ridge.
   * @param painter Display support.
   * @param col Detected ridge display color.
   */
  void displayRidge (ASPainter& painter, ASColor col = ASColor::GREEN);

  /**
   * \brief Displays connected bumps of detected ridge.
   * @param painter Display support.
   * @param col Detected ridge display color.
   */
  void displayConnectedRidge (ASPainter& painter, ASColor col = ASColor::GREEN);

  /**
   * \brief Selects a detection stroke.
   */
  void selectStroke (Pt2i pt);

  /**
   * \brief Draws a carriagetrack plateau.
   * @param painter Display support.
   * @param ct Carriage track which owns the plateau to display.
   * @param num Plateau number.
   * @param rev Scan direction wrt input stroke.
   * @param pt0 Former plateau start point.
   * @param pt1 Former plateau end point.
   * @param miss Count of undisplayed plateaux.
   * @param slast Last valid start position.
   * @param elast Last valid end position.
   * @param pp1 Input stroke start point.
   * @param p12 Input stroke vector.
   * @param l12 Input stroke length.
   */
  bool displayConnectedPlateau (ASPainter& painter, CarriageTrack* ct, int num,
                                bool rev, Pt2i& pt0, Pt2i& pt1,
                                int& miss, float& slast, float& elast,
                                Pt2i pp1, Vr2i p12, float l12);

  /**
   * \brief Draws a ridge bump.
   * @param painter Display support.
   * @param rdg Ridge which owns the bump to display.
   * @param num Bump number.
   * @param rev Scan direction wrt input stroke.
   * @param pt0 Former bump start point.
   * @param pt1 Former bump end point.
   * @param miss Count of undisplayed bumps.
   * @param slast Last valid start position.
   * @param elast Last valid end position.
   * @param pp1 Input stroke start point.
   * @param p12 Input stroke vector.
   * @param l12 Input stroke length.
   */
  bool displayConnectedBump (ASPainter& painter, Ridge* rdg, int num, bool rev,
                             Pt2i& pt0, Pt2i& pt1, int &miss,
                             float &slast, float &elast,
	                     Pt2i pp1, Vr2i p12, float l12);

  /**
   * \brief Detects a stored carriage track.
   * Tests if the track contains the selected point.
   * @param pt Selected point.
   */
  bool selectConnectedTrack (Pt2i pt);

  /**
   * \brief Detects a plateau of a stored carriage track.
   * Tests if the plateau contains the selected point.
   * @param pt Selected point.
   * @param ct Carriage track which owns the plateau to display.
   * @param num Plateau number.
   * @param rev Scan direction wrt input stroke.
   * @param pt0 Former plateau start point.
   * @param pt1 Former plateau end point.
   * @param pp1 Input stroke start point.
   * @param p12 Input stroke vector.
   * @param l12 Input stroke length.
   */
  bool selectConnectedPlateau (Pt2i pt, CarriageTrack* ct, int num, bool rev,
                               Pt2i& pt0, Pt2i& pt1,
                               Pt2i pp1, Vr2i p12, float l12);

};
