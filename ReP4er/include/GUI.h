#pragma once
#include "JuceHeader.h"


//Base class for the main component

class P4GuiComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:
	
	P4GuiComponent()
	{
			
	//	txt_pluginVersion->setText(GetPluginVersionString(), juce::NotificationType::dontSendNotification);
		statusLabel->setText("ReP4er - Settings",juce::NotificationType::dontSendNotification);
		addAndMakeVisible(statusLabel);

		setSize(500, 500);

	};
	~P4GuiComponent()
	{
	};


	juce::Label * statusLabel = new Label();
	
//	juce::Label * txt_pluginVersion = new Label();
	
//	juce::HyperlinkButton * helpButton = new HyperlinkButton ("Help", {"https://simongumbleton.github.io/CSGReaperWwisePlugin/" });
	
//	juce::Button* p4Login = new TextButton("Login");

//	juce::Button* p4CheckoutProjDir = new TextButton("Checkout");

//	juce::Button* p4ReconcileProjDir = new TextButton("Reconcile");


	void setStatusText(std::string message)
	{
		statusLabel->setText("Status: "+message, juce::NotificationType::dontSendNotification);
	}
	
	void handleCommandMessage (int commandId) override
	{
	}
	
	void resized()
	{
		auto area = getBoundsInParent();
		statusLabel->setBounds(area.removeFromTop(20));
		statusLabel->setJustificationType(Justification::centred);
	};

	void buttonClicked(juce::Button* pButton)
	{

	};

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
	{

	};

	void labelTextChanged(Label* labelThatHasChanged)
	{

	};


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(P4GuiComponent)
};

class P4GuiWindow : public juce::DocumentWindow
{
	bool* mWindowState;
	juce::Component* component;
public:
	P4GuiWindow(const juce::String& name, bool* windowStatus) : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
		juce::DocumentWindow::allButtons)
	{
		mWindowState = windowStatus;
		*mWindowState = true;
		setUsingNativeTitleBar(true);
		component = new P4GuiComponent();
		setContentOwned(component, true);

		setResizable(true, false);
		setResizeLimits(300, 250, 10000, 10000);
		centreWithSize(getWidth(), getHeight());

		setVisible(true);
	}

	void closeButtonPressed() override
	{
		*mWindowState = false;
		delete this;
	}

private:


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(P4GuiWindow)
};