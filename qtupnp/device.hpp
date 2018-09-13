#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "service.hpp"
#include "devicepixmap.hpp"
#include "datacaller.hpp"
#include <QSharedDataPointer>
#include <QUrl>

START_DEFINE_UPNP_NAMESPACE

/*! Services map.
 * \param QString: Service name.
 * \param CService: Service data.
 */
typedef QMap<QString, CService> TMServices;

/*! Forward declaration of service data. */
class SDeviceData;

/*! \brief The CDevice class holds information about a device being advertised or found by a control point.
 *
 * It maintains a list of services and embedded devices if any.
 *
 * \remark Use implicit Sharing QT technology.
 */
class UPNP_API CDevice
{
public:
  /*! Type of device. */
  enum EType { Unknown,  //!< Unknown device.
               MediaServer, //!< Media server.
               MediaRenderer, //!< Media renderer.
               DimmableLight, //!< Dimmable light.
               Printer, //!< Printer.
               Basic, //!< Basic device with no service.
               InternetGateway, //!< internet gateway.
               WANDevice, //!< Wan device. Generally a embeded device of InternetGateway
               WANConnectionDevice, //!< Wan connection device. Generally an embedded device of WANDevice
             };

  /*! Type of application prefered pixmap size. */
  enum EPreferedPixmapType { SmResol, //!< Small resolution.
                             Nearest, //!< Nearest resolution.
                             HiResol, //!< High resolution.
                           };

  enum EPlaylistStatus { UnknownHandler = -1, //!< The device can use playlists status unknown.
                         NoPlaylistHandler = 0, //!< The device cannot use playlists.
                         PlaylistHandler = 1, //!< The device can use playlists.
                       };

  /*! Default constructor. */
  CDevice ();

  /*! Copy constructor. */
  CDevice (CDevice const &);

  /*! Equal operator. */
  CDevice& operator = (CDevice const &);

  /*! Destructor. */
  ~CDevice ();

  /*! Uses by renderer. Indicates that the device can manage playlist. */
  void setPlaylistStatus (EPlaylistStatus state);

  /*! Sets the device type. */
  void setType (EType type);

  /*! Initializes the device type from xml description. */
  void setType ();

  /*! Sets the uuid. */
  void setUUID (QString const & uuid);

  /*! Sets the url. */
  void setURL (QUrl const & url);

  /*! Sets the model name. */
  void setModelName (QString const & name);

  /*! Sets the model number. */
  void setModelNumber (QString const & number);

  /*! Sets the model url. */
  void setModelURL (QString const & url);

  /*! Sets the model description. */
  void setModelDesc (QString const & desc);

  /*! Sets the serial number. */
  void setSerialNumber (QString const & number);

  /*! Sets the UPC. */
  void setUpc (QString const & upc);

  /*! Sets the presentation url. */
  void setPresentationURL (QString const & url);

  /*! Sets the manufacturer. */
  void setManufacturer (QString const & manufacturer);

  /*! Sets the manufacturer url. */
  void setManufacturerURL (QString const & url);

  /*! Sets the friendly name. */
  void setFriendlyName (QString const & name);

  /*! Sets the device type. */
  void setDeviceType (QString const & type);

  /*! Sets the DLNA docs. */
  void setDlnaDocs (QStringList const & docs);

  /*! Adds the DLNA doc. */
  void addDlnaDoc (QString const & doc);

  /*! Sets the DLNA capabilities. */
  void setDlnaCaps (QStringList const & caps);

  /*! Initializes a new image */
  void addPixmap ();

  /*! Sets the pixmap device list. */
  void setPixmaps (QList<CDevicePixmap> const & pixmaps);

  /*! Sets the major version. */
  void setMajorVersion (unsigned version);

  /*! Sets the minor version. */
  void setMinorVersion (unsigned version);

  /*! Sets the connection identifier.
   * \param id: Generaly the identifier returned by GetCurrentConnectionIDs action.
   */
  void setConnectionID (unsigned id);

  /*! Sets the services. */
  void setServices (TMServices const & services);

  /*! Sets the parent uuid in case of embedded device. */
  void setParentUUID (QString const & uuid);

  /*! Returns true if the device is an embedded device. */
  bool isSubDevice () const;

  /*! Returns true if the device contains an embedded device. */
  bool hasSubDevice (QString const & uuid) const;

  /*! Returns the embedded device object.
   * \param uuid: Device uuid.
   * \return The device object.
   */
  CDevice subDevice (QString const & uuid) const;

  /*! Returns true if protocol has found.
   * \param protocol: Protocol to search.
   * \param exact: Search the exact protocol.
   * The protocol is valid only if "urn:upnp-org:serviceId:ConnectionManager" is correctly evented
   * and if the device has subcripted at this service.
   *
   * E.g. "http-get:*:audio/x-mpegurl:*" generally can be search with protocol = "audio/x-mpegurl" and
   * exact = "false".
   */
  bool hasProtocol (QString const & protocol, bool exact = false) const;

  /*! Returns the playlist status.
   * The playlist status is valid only if urn:upnp-org:serviceId:ConnectionManager is correctly evented
   * and if the device has subcripted at this service.
   */
  EPlaylistStatus playlistStatus () const;

  /*! Returns the device type. */
  EType type () const;

  /*! Returns the uuid. */
  QString const & uuid () const;

  /*! Returns the url. */
  QUrl const & url () const;

  /*! Returns the model name. */
  QString const & modelName () const;

  /*! Returns the model number. */
  QString const & modelNumber () const;

  /*! Returns the model url. */
  QString const & modelURL () const;

  /*! Returns the model description. */
  QString const & modelDesc () const;

  /*! Returns the serial number. */
  QString const & serialNumber () const;

  /*! Returns the UPC. */
  QString const & upc () const;

  /*! Returns the presentation url. */
  QString const & presentationURL () const;

  /*! Returns the manufacturer. */
  QString const & manufacturer () const;

  /*! Returns the manufacturer url. */
  QString const & manufacturerURL () const;

  /*! Returns the friendly name. */
  QString const & friendlyName () const;

  /*! Returns the device type. */
  QString const & deviceType () const;

  /*! Returns the DLNA docs. */
  QStringList const & dlnaDocs () const;

  /*! Returns the DLNA capabilities. */
  QStringList const & dlnaCaps () const;

  /*! Returns the pixmap device list. */
  QList<CDevicePixmap> const & pixmaps () const;

  /*! Returns the pixmap device list. */
  QList<CDevicePixmap>& pixmaps ();

  /*! Returns the version. */
  unsigned version () const;

  /*! Returns the major version. */
  unsigned majorVersion () const;

  /*! Returns the minor version. */
  unsigned minorVersion () const;

  /*! Returns rhe highest supported version of the device. */
  unsigned highestSupportedVersion () const;

  /*! Returns the connection identifier.
   * \return id: Identifier returned by GetCurrentConnectionIDs action.
   */
  unsigned connectionID () const;

  /*! Returns the services as a const reference. */
  TMServices const & services () const;

  /*! Returns the services as a reference. */
  TMServices& services ();

  /*! Returns the embeded device as a const reference. */
  QList<CDevice> const & subDevices () const;

  /*! Returns the embedded device as a reference. */
  QList<CDevice>& subDevices ();

  /*! Returns the parent uuid. */
  QString const & parentUUID () const;

  /*! Returns the url of the pixmap.
   * \param type: Pixmap type.
   * \param mimeType: The mime type.
   * \param width: The prefered widht or the nearest width.
   * \param height: The prefered widht or the nearest height.
   */
  QUrl pixmap (EPreferedPixmapType type = HiResol, char const * mimeType = "jpeg", int width = 0, int height = 0) const;

  /*! Returns the pixmap bytes. The bytes can be used by QPixmap::loadFromData.
   * \param naMgr: A QNetworkAccessManager.
   * \param type: Pixmap type.
   * \param mimeType: The mime type.
   * \param width: The prefered widht or the nearest width.
   * \param height: The prefered widht or the nearest height.
   * \param timeout: The timeout of the request.
   */
  QByteArray pixmapBytes (QNetworkAccessManager* naMgr, EPreferedPixmapType type = HiResol,
                          char const * mimeType = "jpeg", int width = 0, int height = 0,
                          int timeout = CDataCaller::Timeout) const;

  /*! Returns the device name.
   * If the friendy name is empty, the modelName is used. If modelName is empty, the url is used.
   */
  QString name () const;

  /*! Returns true if at least a service has subscribed. */
  bool hasSubscribed () const;

  /*! Returns the list of subscribed services. */
  QList<QString> subscribedServices () const;

  /*! Returns the CStateVariable class.
   * \param name: The variable name.
   * \param serviceID: The service identifier.
   * \return The state variable.
   */
  CStateVariable stateVariable (QString const & name, QString const & serviceID) const;

  /*! Returns the CAction class.
   * \param serviceID: The service identifier.
   * \param name: The action name.
   * \return The action.
   */
  CAction action (QString const & serviceID, QString const & name) const;

  /*! Returns true if the device has the action named name.
   * \param serviceID: The service identifier.
   * \param name: The action name.
   * \return True in case of success.
   */
  bool hasAction (QString const & serviceID, QString const & name) const;

   /*! Returns the list of service identifiers for an action.
   * \param actionName: The action name.
   * \return The list of service identifiers.
   */
  QStringList serviceIDs (QString const & actionName) const;

  /*! Returns true if device xml data are correctly parsed. */
  bool parseXml (QByteArray const & data);

  /*! Extracts the services components. */
  bool extractServiceComponents (QNetworkAccessManager* naMgr, int timeout = CDataCaller::Timeout);

  /*! Sets disabled the subscribtion for a list of services.
   * Assume a renderer have two no standard services named:
   * - urn:schemas-company-com:serviceId:X_ServiceManager
   * - urn:schemas-company-com:serviceId:X_GroupService
   *
   * These service identifiers can be retreave using eventedServices function.
   *
   * Assume also your have a slot newDevice connected at the newDevice signal of the control point (cp).
   * In this slot the code below disable the subscribtion at these services.
   * \code
   * void YourClass::newDevice (QString const & uuid)
   * {
   *   CDevice& device = cp->device (uuid);
   *   .....
   *   CDevice::EType type = device.type ();
   *   if (type == CDevice::MediaRenderer)
   *   {
   *     QStringList ids;
   *     ids <<  "urn:schemas-company-com:serviceId:X_ServiceManager";
   *     ids << "urn:schemas-company-com:serviceId:X_GroupService";
   *     d.setSubscribtionDisabled (serviceIds);
   *   }
   * }
   * \endcode
   *
   * The function CControlPoint::subscribe will skip these services for subscribtion.
   * you will not receive UPnP events from these services.
   */
  void setSubscribtionDisabled (QStringList const & ids, bool disable = true);

  /*! Returns the list of services which support eventing. */
  QStringList eventedServices () const;

  /*! Replaces all subdevices by the content of devices.
   * Normally you never need to call this function.
   */
  void replaceSubDevices (QList<CDevice> const & devices);

private:
  QSharedDataPointer<SDeviceData> m_d; //!< Shared data pointer.
};

} // Namespace

#endif // DEVICE_HPP
