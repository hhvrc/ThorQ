namespace ThorQ
{
    partial class LoginForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LoginForm));
			this.LoginButton = new System.Windows.Forms.Button();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.RegistrationButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.ForgotPasswordButton = new System.Windows.Forms.Button();
			this.OptionsButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// LoginButton
			// 
			this.LoginButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
			this.LoginButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(40)))), ((int)(((byte)(40)))));
			this.LoginButton.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(20)))), ((int)(((byte)(20)))), ((int)(((byte)(20)))));
			this.LoginButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.LoginButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 15F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.LoginButton.Location = new System.Drawing.Point(12, 244);
			this.LoginButton.Name = "LoginButton";
			this.LoginButton.Size = new System.Drawing.Size(388, 55);
			this.LoginButton.TabIndex = 0;
			this.LoginButton.Text = "Login";
			this.LoginButton.UseVisualStyleBackColor = false;
			this.LoginButton.Click += new System.EventHandler(this.LoginButton_Click);
			// 
			// textBox1
			// 
			this.textBox1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(40)))), ((int)(((byte)(40)))));
			this.textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.textBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 20F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.textBox1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
			this.textBox1.Location = new System.Drawing.Point(12, 88);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(388, 31);
			this.textBox1.TabIndex = 1;
			this.textBox1.Text = "test";
			// 
			// textBox2
			// 
			this.textBox2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(40)))), ((int)(((byte)(40)))));
			this.textBox2.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.textBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.textBox2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
			this.textBox2.Location = new System.Drawing.Point(12, 184);
			this.textBox2.Name = "textBox2";
			this.textBox2.Size = new System.Drawing.Size(388, 38);
			this.textBox2.TabIndex = 2;
			this.textBox2.Text = "password";
			this.textBox2.UseSystemPasswordChar = true;
			// 
			// RegistrationButton
			// 
			this.RegistrationButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
			this.RegistrationButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.RegistrationButton.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(20)))), ((int)(((byte)(20)))), ((int)(((byte)(20)))));
			this.RegistrationButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.RegistrationButton.Location = new System.Drawing.Point(12, 305);
			this.RegistrationButton.Name = "RegistrationButton";
			this.RegistrationButton.Size = new System.Drawing.Size(184, 52);
			this.RegistrationButton.TabIndex = 3;
			this.RegistrationButton.Text = "Register";
			this.RegistrationButton.UseVisualStyleBackColor = false;
			this.RegistrationButton.Click += new System.EventHandler(this.RegistrationButton_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
			this.label1.Location = new System.Drawing.Point(9, 63);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(76, 13);
			this.label1.TabIndex = 4;
			this.label1.Text = "USERNAME";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
			this.label2.Location = new System.Drawing.Point(12, 159);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(78, 13);
			this.label2.TabIndex = 5;
			this.label2.Text = "PASSWORD";
			// 
			// ForgotPasswordButton
			// 
			this.ForgotPasswordButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
			this.ForgotPasswordButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.ForgotPasswordButton.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(20)))), ((int)(((byte)(20)))), ((int)(((byte)(20)))));
			this.ForgotPasswordButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.ForgotPasswordButton.Location = new System.Drawing.Point(202, 305);
			this.ForgotPasswordButton.Name = "ForgotPasswordButton";
			this.ForgotPasswordButton.Size = new System.Drawing.Size(198, 52);
			this.ForgotPasswordButton.TabIndex = 6;
			this.ForgotPasswordButton.Text = "Forgotten your password?";
			this.ForgotPasswordButton.UseVisualStyleBackColor = false;
			this.ForgotPasswordButton.Click += new System.EventHandler(this.ForgotPasswordButton_Click);
			// 
			// OptionsButton
			// 
			this.OptionsButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
			this.OptionsButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.OptionsButton.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(20)))), ((int)(((byte)(20)))), ((int)(((byte)(20)))));
			this.OptionsButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.OptionsButton.Location = new System.Drawing.Point(-2, -1);
			this.OptionsButton.Name = "OptionsButton";
			this.OptionsButton.Size = new System.Drawing.Size(58, 25);
			this.OptionsButton.TabIndex = 7;
			this.OptionsButton.Text = "Options";
			this.OptionsButton.UseVisualStyleBackColor = false;
			this.OptionsButton.Click += new System.EventHandler(this.OptionsButton_Click);
			// 
			// LoginForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.ClientSize = new System.Drawing.Size(409, 368);
			this.Controls.Add(this.OptionsButton);
			this.Controls.Add(this.ForgotPasswordButton);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.RegistrationButton);
			this.Controls.Add(this.textBox2);
			this.Controls.Add(this.textBox1);
			this.Controls.Add(this.LoginButton);
			this.DoubleBuffered = true;
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "LoginForm";
			this.Text = "Login";
			this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.OnClosed);
			this.ResumeLayout(false);
			this.PerformLayout();

        }

		#endregion

		private System.Windows.Forms.Button LoginButton;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.TextBox textBox2;
		private System.Windows.Forms.Button RegistrationButton;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button ForgotPasswordButton;
		private System.Windows.Forms.Button OptionsButton;
	}
}

