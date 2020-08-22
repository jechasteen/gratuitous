#include "highlighter.h"

#include <QList>

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Names/Identifiers
    keywordFormat.setForeground(Qt::cyan);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setFontItalic(true);
    const QList<QString> keywordPatterns =
    {
        QStringLiteral("\\band\\b"), QStringLiteral("\\bbreak\\b"),
        QStringLiteral("\\bdo\\b"), QStringLiteral("\\belse\\b"),
        QStringLiteral("\\belseif\\b"), QStringLiteral("\\bend\\b"),
        QStringLiteral("\\bfalse\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bfunction\\b"), QStringLiteral("\\bif\\b"),
        QStringLiteral("\\bin\\b"), QStringLiteral("\\blocal\\b"),
        QStringLiteral("\\bnil\\b"), QStringLiteral("\\bnot\\b"),
        QStringLiteral("\\bor\\b"), QStringLiteral("\\brepeat\\b"),
        QStringLiteral("\\breturn\\b"), QStringLiteral("\\bthen\\b"),
        QStringLiteral("\\btrue\\b"), QStringLiteral("\\buntil\\b"),
        QStringLiteral("\\bwhile\\b")
    };
    for (const QString &pattern : keywordPatterns)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    quotationFormat.setForeground(Qt::darkYellow);
    quotationFormat.setFontWeight(QFont::Bold);
    const QList<QString> stringPatterns =
    {
        QStringLiteral("\"[^\"]*\"")
    };
    for (const QString &pattern : stringPatterns)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = quotationFormat;
        highlightingRules.append(rule);
    }

    // Single-line comment
    singleLineCommentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression(QStringLiteral("--[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::gray);
    commentStartExpression = QRegularExpression(QStringLiteral("--\\[\\["));
    commentEndExpression = QRegularExpression(QStringLiteral("--\\]\\]"));
}

void Highlighter::highlightBlock(const QString &text)
{
    // Do basic highlighting (single-line)
    for (const HighlightingRule &rule : qAsConst(highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Get the multi-line comments
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0)
    {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
