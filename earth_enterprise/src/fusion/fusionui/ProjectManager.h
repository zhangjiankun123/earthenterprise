/*
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef KHSRC_FUSION_FUSIONUI_PROJECTMANAGER_H__
#define KHSRC_FUSION_FUSIONUI_PROJECTMANAGER_H__

#include <qlistview.h>
#include <qfiledialog.h>
#include <qdragobject.h>
#include <vector>

#include <limits.h>
#include <notify.h>
#include <khArray.h>
#include <gstBBox.h>
#include <gstTypes.h>
#include <gstGeode.h>

#include "AssetWidgetBase.h"
#include <autoingest/.idl/storage/LayerConfig.h>

class AssetBase;
class QGroupBox;
class QLineEdit;
class QPainter;
class gstDrawState;
class gstVectorProject;
class gstSource;
class gstLayer;
class gstFilter;
class gstSelector;
class LayerProperties;
class SelectionRules;
class SelectionView;
class ProjectDocker;
class ProjectManagerHolder;
class VectorProjectEditRequest;

//
// The ProjectManager can operate in two modes
//    1. Preview
//    2. Vector Project
//
//    getType() will return an emum of either:
//      ProjectManager::Preview or ProjectManager::Project
//


// used to distinguish which type of listview item
// was clicked on during run-time
// NOTE: QT wants these to be greater than 1000
#define LAYER      0x00ff0001
#define FILTER     0x00ff0002
#define GROUP      0x00ff0003
//#define DATABASE   0x00ff0004

class ProjectManager;
class LayerItem;


// -----------------------------------------------------------------------------

class ProjectManager : public QListView {
  Q_OBJECT

 public:
  enum Type { Preview, Project };

  ProjectManager(QWidget* parent, const char* name, Type t);
  ~ProjectManager(void);

  void addLayers(const char* src, const char* codec);


  gstVectorProject* getProject() const { return project_; }

  bool applyQueries(gstLayer* l);

  Type getType() const { return type_; }

  void cancelDrag();

  static QString CleanupDropText(const QString& t);

  void forcePreviewRedraw();

  uint numLayers();

  void UpdateWidgets();

 protected:
  friend class ProjectDocker;

  void enableAllLayers(bool m);
  void openAllLayers(bool m);

  void updateLayerItem(LayerItem* item);

  void paintEvent(QPaintEvent* e);

  gstLayer* getSelectedLayer();

  // drag/drop support
  // inherited from qlistview
  virtual void contentsMousePressEvent(QMouseEvent* e);
  virtual void contentsMouseMoveEvent(QMouseEvent* e);
  virtual void contentsMouseReleaseEvent(QMouseEvent* e);
  // inherited from qscrollview
  virtual void contentsDragMoveEvent(QDragMoveEvent* e);
  virtual void contentsDragLeaveEvent(QDragLeaveEvent* e);
  virtual void contentsDropEvent(QDropEvent* e);

 signals:
  void zoomToSource(const gstBBox& b);
  void statusMessage(const QString& m);
  void selectionView(gstSelector* s);
  void layerStateChange(int, int, int);
  void redrawPreview();
  void selectLayer(gstLayer* l);
  void showSearchBox(bool);

 public slots:
  void FileOpen();

 protected slots:
  void contextMenu(QListViewItem* i, const QPoint& pos, int);
  void itemDoubleClicked(QListViewItem* i);
  void selectBox(const gstDrawState& state, Qt::ButtonState b);

  void DrawFeatures(const gstDrawState& state);
  void DrawLabels(QPainter* p, const gstDrawState& state);

  void removeLayer(QListViewItem* a = NULL);
  void moveLayerUp(QListViewItem* item = NULL);
  void moveLayerDown(QListViewItem* item = NULL);

  void makeTopLevel(LayerItem* i);

  void selectItem(QListViewItem* i);
  void addLayer();
  void addLayerGroup();

  void pressed(QListViewItem* item);
  void selectionChanged(QListViewItem* item);

  // from QWidget
  virtual void customEvent(QCustomEvent*);
 private:
  void DrawEditBuffer(const gstDrawState& state);
  GeodeList edit_buffer_;

  void exportDisplayTemplate(gstLayer* layer);
  void importDisplayTemplate(QCheckListItem* item);
  void configureDisplayRules(LayerItem* layer, int id);
  LayerConfig MakeDefaultLayerConfig(const QString &name,
                                     gstPrimType primType,
                                     const std::string &assetRef) const;
  LayerConfig MakeDefaultGroupConfig(const QString &name) const;
  void AddAssetLayer(const char* assetname);
  gstLayer* CreateNewLayer(const QString& layername,
                           gstSource* src, int layer,
                           const std::string &assetRef);
  gstSource* openSource(const char* src, const char* codec,
                        bool nofileok);
  void updateButtons(QListViewItem* item);
  bool canRaise(QListViewItem* item);
  bool canLower(QListViewItem* item);
  void RefreshLayerList(bool setLegends, bool setSortIds);
  bool EnsureUniqueLayerName(QListViewItem* item);
  bool FindLayerNameAmongSiblings(QListViewItem* item, const QString& name);
  bool LayerNameSanityCheck(QListViewItem *item, const QString& name);
  bool FindUuid(QListViewItem* parent, QListViewItem* item, const QString& uuid);
  bool UuidSanityCheck(QListViewItem* item,
                           const std::string &old_uuid,
                           std::string &new_uuid);
  void removeAllLayers(void);
  void ClearSelection();

  bool validateSrs(gstSource* src);

  gstVectorProject* project_;

  Type type_;

  // drag/drop support
  QPoint press_pos_;
  bool mouse_pressed_;
  LayerItem* drag_layer_;
  QListViewItem* old_current_;

  bool show_max_count_reached_message_;
};

// -----------------------------------------------------------------------------

class ProjectManagerHolder : public QWidget,
                             public AssetWidgetBase {
 Q_OBJECT

 public:
  ProjectManagerHolder(QWidget* parent, AssetBase* base);

  ProjectManager* GetProjectManager() { return project_manager_; }

  void Prefill(const VectorProjectEditRequest &req);
  void AssembleEditRequest(VectorProjectEditRequest *request);

 protected slots:
  void UpdateLayerControls(int, int, int);

 private:
  QLabel* proj_name_;
  QPushButton* new_layer_;
  QPushButton* delete_layer_;
  QPushButton* new_layer_group_;
  QPushButton* up_layer_;
  QPushButton* down_layer_;

  ProjectManager* project_manager_;
};

#endif  // !KHSRC_FUSION_FUSIONUI_PROJECTMANAGER_H__
