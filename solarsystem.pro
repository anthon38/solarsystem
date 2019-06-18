TEMPLATE = app
TARGET   = solarsystem

QT += gui quick qml

HEADERS +=  \
    body.h \
    timeline.h \
    renderable/axis.h \
    renderable/galaxy.h \
    renderable/orbit.h \
    renderable/renderable.h \
    renderable/ring.h \
    renderable/sphere.h \
    viewitem.h \
    path.h \
    osdetails.h \
    renderable/pointobject.h \
    camera.h \
    renderable/textbillboard.h \
    renderable/pickable.h \
    renderable/screenquad.h \
    renderable/flare.h

SOURCES +=  \
    body.cpp \
    main.cpp \
    timeline.cpp \
    renderable/axis.cpp \
    renderable/galaxy.cpp \
    renderable/orbit.cpp \
    renderable/renderable.cpp \
    renderable/ring.cpp \
    renderable/sphere.cpp \
    viewitem.cpp \
    osdetails.cpp \
    renderable/pointobject.cpp \
    camera.cpp \
    renderable/textbillboard.cpp \
    renderable/pickable.cpp \
    renderable/screenquad.cpp \
    renderable/flare.cpp

OTHER_FILES += \
    android/AndroidManifest.xml \
    data/data.txt \
    qml/main.qml \
    shadersES2/blur.frag \
    shadersES2/body.frag \
    shadersES2/combineTexture.frag \
    shadersES2/earth.frag \
    shadersES2/galaxy.frag \
    shadersES2/orbit.frag \
    shadersES2/ring.frag \
    shadersES2/solidColor.frag \
    shadersES2/sun.frag \
    shadersES2/blur.vert \
    shadersES2/body.vert \
    shadersES2/combineTexture.vert \
    shadersES2/earth.vert \
    shadersES2/galaxy.vert \
    shadersES2/orbit.vert \
    shadersES2/ring.vert \
    shadersES2/solidColor.vert \
    shadersES2/sun.vert \
    shadersES2/simpleTexture.frag \
    shadersES2/simpleTexture.vert \
    shadersES2/coloredPoint.frag \
    shadersES2/pointSolidColor.frag \
    shadersES2/coloredPoint.vert \
    shadersES2/pointSolidColor.vert \
    qml/CustomCheckBox.qml \
    qml/CustomRadioButton.qml \
    qml/Style.qml \
    qml/qmldir \
    qml/CustomText.qml \
    qml/CustomSlider.qml \
    shadersES2/axis.vert \
    shadersES2/axis.frag \
    shadersES2/textBillboard.frag \
    shadersES2/textBillboard.vert \
    shadersES2/billboardSolidColor.frag \
    shadersES2/billboardSolidColor.vert \
    shadersES2/flare.frag \
    shadersES2/flare.vert \
    shadersES2/fxaa.frag \
    shadersES2/fxaa.vert \
    shadersES2/FXAA.frag \
    shadersES2/FXAA.vert \
    shadersES2/FXAA3.11.frag \
    shadersES2/FXAA3.11.vert

RESOURCES +=

android {
    assets.path = /assets
    assets.files = data icons qml shadersES2 textures
    INSTALLS += assets
}

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

DISTFILES += \
    README.md
