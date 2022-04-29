#pragma once
#include "JUCE/JuceHeader.h"


/** Just a simple window that deletes itself when closed. */
class BasicWindow : public DocumentWindow
{
public:
	BasicWindow(const String& name, Colour backgroundColour, int buttonsNeeded)
		: DocumentWindow(name, backgroundColour, buttonsNeeded)
	{
	}

	void closeButtonPressed()
	{
		delete this;
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicWindow)
};

//==============================================================================
class MainContentComponent : public juce::Component,public juce::Button::Listener
{
public:
	MainContentComponent()
	{
		dial1.setSliderStyle(juce::Slider::Rotary);
		dial1.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
		addAndMakeVisible(dial1);

		dial2.setSliderStyle(juce::Slider::Rotary);
		dial2.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
		addAndMakeVisible(dial2);

		button1.setButtonText("Button 1");
		button1.addListener(this);
		addAndMakeVisible(button1);

		button2.setButtonText("Button 2");
		button2.addListener(this);
		addAndMakeVisible(button2);

		label.setText("label",juce::NotificationType::dontSendNotification);
		addAndMakeVisible(label);


		setSize(300, 200);
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}

	void resized() override
	{
		auto border = 4;

		auto area = getLocalBounds();

		auto dialArea = area.removeFromTop(area.getHeight() / 2);
		dial1.setBounds(dialArea.removeFromLeft(dialArea.getWidth() / 2).reduced(border));
		dial2.setBounds(dialArea.reduced(border));

		auto buttonHeight = 30;

		button1.setBounds(area.removeFromTop(buttonHeight).reduced(border));
		button2.setBounds(area.removeFromTop(buttonHeight).reduced(border));

		auto labelHeight = 20;
		label.setBounds(area.removeFromBottom(labelHeight));
	}

	void buttonClicked(juce::Button* pButton) override
	{
		pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
		String text = ("CLICKED: " + pButton->getButtonText());
		label.setText(text, juce::NotificationType::dontSendNotification);
	}

private:
	juce::Slider dial1;
	juce::Slider dial2;
	juce::TextButton button1;
	juce::TextButton button2;
	juce::Label label;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

class MainWindow : public juce::DocumentWindow
{
public:
	MainWindow(const juce::String& name, juce::Component* c) : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
			juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(c, true);

		setResizable(true, false);
		setResizeLimits(300, 250, 10000, 10000);
		centreWithSize(getWidth(), getHeight());

		setVisible(true);
	}

	void closeButtonPressed() override
	{
		delete this;
	}

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

using namespace CreateTransfer;
//Base class for the main component for transfer behaviour to wwise

class BaseWwiseGuiComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:
	
	BaseWwiseGuiComponent()
	{
		txt_pluginVersion->setText(GetPluginVersionString(), juce::NotificationType::dontSendNotification);
	};
	~BaseWwiseGuiComponent()
	{
		
	};
	
	double transferProgress = 0.0f;
	juce::ProgressBar * progressBar = new ProgressBar(transferProgress);
	
	juce::Label * statusLabel = new Label();
	
	juce::Label * txt_pluginVersion = new Label();
	
	juce::HyperlinkButton * helpButton = new HyperlinkButton ("Help", {"https://simongumbleton.github.io/CSGReaperWwisePlugin/" });
	
	void setStatusText(std::string message)
	{
		statusLabel->setText("Status: "+message, juce::NotificationType::dontSendNotification);
	}

	void updateProgressValue(double newProgress)
	{
		transferProgress = newProgress;
		this->repaint();
	}
	
	void handleCommandMessage (int commandId) override
	{
	}
	
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseWwiseGuiComponent)
};
