#pragma once

#include <QRadioButton>
#include <QIcon>
#include <QPainter>

class IconRadioButton : public QRadioButton {
    Q_OBJECT

public:
    explicit IconRadioButton(QWidget *parent = nullptr)
        : QRadioButton(parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(this, &QRadioButton::toggled, this, QOverload<>::of(&IconRadioButton::update));
    }

    void setIcons(const QIcon &checkedIcon, const QIcon &uncheckedIcon, const QSize &iconSize) {
        m_checkedIcon = checkedIcon;
        m_uncheckedIcon = uncheckedIcon;

        if (!iconSize.isEmpty()) {
            m_iconSize = iconSize;
            setFixedSize(iconSize);
        }

        update();
    }

    void setTextColors(const QColor &checkedColor, const QColor &uncheckedColor) {
        m_checkedTextColor = checkedColor;
        m_uncheckedTextColor = uncheckedColor;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QIcon icon = isChecked() ? m_checkedIcon : m_uncheckedIcon;
        if (!icon.isNull()) {
            icon.paint(&painter, rect());
        }

        if (!text().isEmpty()) {
            QColor textColor = isChecked() ? m_checkedTextColor : m_uncheckedTextColor;
            painter.setPen(textColor.isValid() ? textColor : palette().windowText().color());
            painter.setFont(font());
            painter.drawText(rect(), Qt::AlignCenter, text());
        }
    }

    bool hitButton(const QPoint &pos) const override {
        return rect().contains(pos);
    }

private:
    QIcon m_checkedIcon;
    QIcon m_uncheckedIcon;
    QSize m_iconSize;

    QColor m_checkedTextColor;
    QColor m_uncheckedTextColor;
};
